const scalerNum = 32;

// mode chagne
const historyButton = document.getElementById('history');
const realtimeButton = document.getElementById('realtime');
historyButton.onclick = () => selectMode(0);
realtimeButton.onclick = () => selectMode(1);

// history date selection
const startDateInput = document.getElementById('start-date');
startDateInput.onchange = () => console.log(startDateInput.value);
const endDateInput = document.getElementById('end-date');
endDateInput.onchange = () => console.log(endDateInput.value);
let nowTime = new Date();
const offset = nowTime.getTimezoneOffset();
nowTime = new Date(nowTime.getTime() - offset*60*1000);
const inputMaxTime = nowTime.toISOString().split('T')[0];
startDateInput.max = `${inputMaxTime}`;
endDateInput.max = `${inputMaxTime}`;
// history date labels
const startDateLabel = document.getElementById('start-date-label');
const endDateLabel = document.getElementById('end-date-label');
// history date request button
const requestHistoryButton = document.getElementById('request-date');
requestHistoryButton.onclick = setHistoryChart;

// realtime range selection
const timeRangeSelect = document.getElementById('realtime-range-select');
const timeRangeLabel = document.getElementById('realtime-range-label');
timeRangeSelect.onchange = () => setRealtimeChart();

// my chart
var myChart = echarts.init(document.getElementById('chart'), 'vintage');
window.onresize = () => myChart.resize();

// status bar
const statusDisplay = document.getElementById('status');

// scaler value
const showScalerValueButton = document.getElementById('show-scaler-value');
let showScalerValue = false;
showScalerValueButton.onclick = () => {
		if (showScalerValue) {
			showScalerValue = false;
			scalerValueDiv.hidden = true;
			showScalerValueButton.value = "+ show scaler value";
			statusDisplay.innerText = 'hide scaler value';
		} else {
			showScalerValue = true;
			scalerValueDiv.hidden = false;
			showScalerValueButton.value = "- hide scaler value";
			statusDisplay.innerText = 'show scaler value';
		}
}
const scalerValueDiv = document.getElementById('scaler-value');
let scalerDisplay = [];
const scalerDisplayLine = 8;
for (var i = 0; i < scalerDisplayLine; ++i) {
	scalerDisplay.push(document.createElement('pre'));
	scalerValueDiv.appendChild(scalerDisplay[i]);
	scalerDisplay[i].innerText = `line ${i}`;
}

let defaultScalerNames = [];
for (var i = 0; i < scalerNum; ++i) {
	defaultScalerNames.push((i).toString());
}
let legendSelected = [];
for (var i = 0; i < scalerNum; ++i) {
	legendSelected.push(false);
}

let realtimeIntervalId = 0;
function selectMode(mode) {
	if (mode == 0) {
		// history mode
		
		// change buttons appearance
		historyButton.style.backgroundColor = '#29f';
		realtimeButton.style.backgroundColor = '#fff';
		
		// show inputs
		startDateInput.hidden = false;
		endDateInput.hidden = false;
		startDateLabel.hidden = false;
		endDateLabel.hidden = false;
		requestHistoryButton.hidden = false;

		// hide inputs
		timeRangeSelect.hidden = true;
		timeRangeLabel.hidden = true;
		

		// set chart
		setHistoryChart();

		// stop realtime interval
		if (realtimeIntervalId != 0) {
			clearInterval(realtimeIntervalId);
			realtimeIntervalId = 0;
		}		

		// hide sclaer value and button
		showScalerValueButton.hidden = true;
		scalerValueDiv.hidden = true;

		// status bar
		statusDisplay.innerText = 'Select history';

	} else {
		// realtime mode

		// change button appearance
		historyButton.style.backgroundColor = '#fff';
		realtimeButton.style.backgroundColor = '#29f';
		
		// show inputs
		timeRangeSelect.hidden = false;
		timeRangeLabel.hidden = false;

		// hide inputs
		startDateInput.hidden = true;
		endDateInput.hidden = true;
		startDateLabel.hidden = true;
		endDateLabel.hidden = true;
		requestHistoryButton.hidden = true;

		// set chart
		setRealtimeChart();

		// show sclaer value and button
		showScalerValueButton.hidden = false;
		scalerValueDiv.hidden = !showScalerValue;

		// status bar
		statusDisplay.innerText = 'Select realtime';
	}
}



function setChart() {
	// Specify the configuration items and data for the chart
	var option = {
		tooltip: {
			trigger: 'axis',
			position: (pt) => [pt[0], '10%']
		},
		toolbox: {
			feature: {
				dataZoom: {
					yAxisIndex: 'none'
				},
				restore: {},
				saveAsImage: {}
			}
		},
		dataZoom: [
			{
				type: "inside",
				start: 0,
				end: 10
			},
			{
				start: 0,
				end: 10
			}
		],
		legend: {
			type: 'scroll',
			orient: 'vertical',
			right: 5,
			top: 50,
			bottom: 50,
			data: [],
			selected: {}
		},
		xAxis: {
			type: 'category',
			boundaryGap: false,
			data: []
		},
		yAxis: {
			type: 'value',
			boundaryGap: [0, '100%']
		},
		series: []
	};
	for (var i = 0; i < scalerNum; i++) {
		option.series.push(
			{
				name: `${defaultScalerNames[i]}`,
				type: 'line',
				symbol: 'none',
				sampling: 'lttb',
				data: []
			}
		)
		option.legend.data.push(`${defaultScalerNames[i]}`);
		option.legend.selected[`${defaultScalerNames[i]}`] = legendSelected[i];
	}

	// Display the chart using the configuration items and data just specified.
	myChart.setOption(option);
}


function updateChartScalerNames(scalerNames) {
	var updateOption = {
		series: [],
		legend: {
			data: [],
			selected: {}
		}
	};
	for (var i = 0; i < scalerNum; i++) {
		updateOption.series.push({
			name: `${scalerNames[i]}`,
			data: []
		})
		updateOption.legend.data.push(`${scalerNames[i]}`);
		updateOption.legend.selected[`${scalerNames[i]}`] = legendSelected[i];	
	}
	// update chart
	myChart.setOption(updateOption);
}


let chartTime = [];
let chartScalers = [];

async function fetchHistoryData() {
	// request scaler values
	const requestContent = {
		"start": startDateInput.value,
		"end": endDateInput.value
	};
	const request = new Request('http://localhost:12308/history',
		{
			method: 'POST',
			body: JSON.stringify(requestContent)
		}
	);
	return fetch(request)
		.then((response) => {
			if (response.status == 200) {
				return response.json();
			} else {
				throw new Error("Something went wrong on server!");
			}
		})
		.then((content) => {
			if (content.status == 0) {
				// fill scaler data
				chartScalers = [];
				for (var i = 0; i < scalerNum; i++) {
					chartScalers.push([]);
					for (value of content.scalers[i]) {
						chartScalers[i].push(value);
					}
				}
			} else {
				throw new Error("Something went wrong in data files!");
			}
			return content.scalers[0].length;
		})
		.catch((error) => {
			console.error(error);
			statusDisplay.innerText = `${error.name}: ${error.message}`;
			return 0;
		})
}

async function setHistoryChart() {
	// check date input
	if (startDateInput.value == "") {
		statusDisplay.innerText = "Please select the start date.";
		return;
	}
	if (endDateInput.value == "") {
		statusDisplay.innerText = "Please select the end date.";
		return;
	}
	if (startDateInput.value > endDateInput.value) {
		statusDisplay.innerText = "End date should after start date.";
		return;
	}

	myChart.showLoading();

	const startDate = new Date(startDateInput.value);
	const startSeconds = startDate.getTime() - 8*3600*1000;
	// const endDate = new Date(endDateInput.value);
	// const dataSize = (endDate.getTime() - startDate.getTime())/1000 + 86400;

	const scalerNames = await getScalerNames();
	updateChartScalerNames(scalerNames);

	// fetch history data from http server
	fetchHistoryData()
		.then((dataSize) => {
			console.log(`history data size ${dataSize}`);
			// initialize the time data
			chartTime = [];
			for (var i = 0; i < dataSize; i++) {
				const calculateTime = new Date(startSeconds + i*1000);
				const month = calculateTime.getMonth() + 1;
				const day = calculateTime.getDate();
				const hour = calculateTime.getHours();
				const minute = calculateTime.getMinutes();
				const second = calculateTime.getSeconds();
				chartTime.push(`${month}-${day} ${hour}:${minute}:${second}`);
			}


			// set updateOption
			var updateOption = {
				xAxis: {
					data: chartTime
				},
				series: []
			};
			for (var i = 0; i < scalerNum; i++) {
				updateOption.series.push({
					name: `${scalerNames[i]}`,
					data: chartScalers[i]
				});
			}

			// set chart
			myChart.setOption(updateOption);
			myChart.hideLoading();
		});	
}


// let chartTime = [];
// let chartScalers = [];
let expectedTime = 0;

async function setRealtimeChart() {
	const timeRange = parseInt(timeRangeSelect.value);
	const nowTime = new Date();
	const nowSecond = parseInt(nowTime.getTime()/1000);
	expectedTime = nowSecond;

	// initialize time data
	chartTime = [];
	for (var i = nowSecond-timeRange; i < nowSecond; i++) {
		const calculateTime = new Date(i*1000);
		// calculateTime.setTime(i*1000);
		const h = calculateTime.getHours();
		const m = calculateTime.getMinutes();
		const s = calculateTime.getSeconds();
		chartTime.push(`${h}:${m}:${s}`);
	}

	// get scaler names
	const scalerNames = await getScalerNames();
	// update scaler namesl
	updateChartScalerNames(scalerNames);
	// console.log(updateOption);
	
	// request scaler values
	const requestContent = {
		"start": nowSecond-timeRange,
		"end": nowSecond
	};
	const request = new Request('http://localhost:12308/realtime',
		{
			method: 'POST',
			body: JSON.stringify(requestContent)
		}
	);
	fetch(request)
		.then((response) => {
			if (response.status == 200) {
				return response.json();
			} else {
				throw new Error("Something went wrong on server!");
			}
		})
		.then((content) => {
			if (content.status == 0) {
				// fill data
				chartScalers = [];
				for (var i = 0; i < scalerNum; i++) {
					chartScalers.push([]);
					for (var j = 0; j < timeRange; j++) {
						chartScalers[i].push(content.scalers[i][j]);
					}
				}

				// update the chart every second
				if (realtimeIntervalId != 0) {
					clearInterval(realtimeIntervalId);
				}
				realtimeIntervalId = setInterval(updateRealtimeChart, 1000, scalerNames);
			} else if (content.status == 2) {
				// delay 100ms
				setTimeout(setRealtimeChart, 100);
			} else {
				throw new Error("Something went wrong in data files!");
			}
		})
		.catch((error) => {
			console.error(error);
			statusDisplay.innerText = `${error.name}: ${error.message}`;
		});
}

function delayUpdateRealtimeChart(delay) {
	clearInterval(realtimeIntervalId);
	realtimeIntervalId = 0;
	setTimeout(() => {
		realtimeIntervalId = setInterval(updateRealtimeChart, 1000);
	}, delay);
}


function updateRealtimeChart(scalerNames) {
	const nowTime = new Date();
	const nowSecond = parseInt(nowTime.getTime()/1000);


	if (nowSecond-1 < expectedTime) {
		// dealy 100ms
		delayUpdateRealtimeChart(100);
	}

	// update last seconds data may be better to avoid time jitter
	let requestContent = {
		"start": expectedTime,
		"end": nowSecond
	};
	expectedTime = nowSecond;

	

	const request = new Request('http://localhost:12308/realtime',
		{
			method: 'POST',
			body: JSON.stringify(requestContent)
		}
	);
	fetch(request)
		.then((response) => {
			if (response.status == 200) {
				return response.json();
			} else {
				throw new Error("Something went wrong on server!");
			}
		})
		.then((content) => {
			if (content.status == 0) {
				// update data
				for (var i = content.scalers[0].length; i > 0; --i) {
					const calculateTime = new Date((nowSecond - i)*1000);
					// calculateTime.setTime((nowSecond - i)*1000);
					chartTime.push(`${calculateTime.getHours()}:${calculateTime.getMinutes()}:${calculateTime.getSeconds()}`);
					chartTime.shift();
				}
				for (var i = 0; i < scalerNum; ++i) {
					for (value of content.scalers[i]) {
						// update scaler value in chart
						chartScalers[i].push(value);
						chartScalers[i].shift();
					}
				}

				// update option
				var updateOption = {
					xAxis: {
						data: chartTime
					},
					series: []
				};
				for (var i = 0; i < scalerNum; i++) {
					updateOption.series.push({
						name: `${scalerNames[i]}`,
						data: chartScalers[i],
					});
				}

				// update chart
				myChart.setOption(updateOption);


				// update scaler value in text
				for (var i = 0; i < scalerDisplayLine; ++i) {
					let lineText = '';
					let lineWidth = parseInt(scalerNum / scalerDisplayLine);
					lineWidth += scalerNum % scalerDisplayLine == 0 ? 0 : 1;
					for (var j = 0; j < lineWidth; ++j) {
						const index = i*lineWidth + j;
						if (index >= scalerNum) {
							continue;
						}
						var str = `${scalerNames[index]}`;
						str = ' '.repeat(8-str.length) + str;
						str += `:  ${content.scalers[index]}`;
						str += ' '.repeat(30-str.length);
						lineText += str;
					}
					scalerDisplay[i].innerText = lineText + "\n"; 
				}

			} else if (content.status == 2) {
				// delay update 100ms
				delayUpdateRealtimeChart(100);
			} else {
				throw new Error("Server can't find this event, something went wrong in data files.");
			}
		})
		.catch((error) => {
			console.error(error);
			statusDisplay.innerText = `${error.name}: ${error.message}`;
		});	
}


async function getScalerNames() {
	const requestContent = {
		"request": "get-scaler-names"
	};
	const request = new Request('http://localhost:12308/settings',
		{
			method: 'POST',
			body: JSON.stringify(requestContent)
		}
	);

	return fetch(request)
		.then((response) => {
			if (response.status == 200) {
				return response.json();
			} else {
				throw new Error("Something went wrong on server!");
			}
		})
		.then((content) => {
			if (content.status == 0) {
				const presentScalerNames = myChart.getOption().legend[0].data;
				const presentLegendSelected = myChart.getOption().legend[0].selected;
				for (var i = 0; i < scalerNum; ++i) {
					legendSelected[i] = presentLegendSelected[`${presentScalerNames[i]}`];
				}
				console.log(legendSelected);
				let scalerNames = [];
				for (var i = 0; i < scalerNum; ++i) {
					scalerNames[i] = content.names[i];
				}
				return scalerNames;
			} else {
				throw new Error("Get scaler names from server failed!");
			}
		})
		.catch((error) => {
			console.error(error);
			statusDisplay.innerText = `${error.name}: ${error.message}`;
			let scalerNames = [];
			for (var i = 0; i < scalerNum; ++i) {
				scalerNames.push(defaultScalerNames[i]);
			}
			return scalerNames;
		})
}


function setScalerNames() {
	let requestContent = {
		"request": "set-scaler-names",
		"names": []
	};
	for (var i = 0; i < scalerNum; ++i) {
		requestContent["names"].push(scalerNames[i]);
	}
	const request = new Request('http://localhost:12308/settings',
		{
			method: 'POST',
			body: JSON.stringify(requestContent)
		}
	);
	fetch(request)
		.then((response) => {
			if (response.status == 200) {
				return response.json();
			} else {
				throw new Error("Something went wrong on server!");
			}
		})
		.then((content) => {
			if (content.status == 0) {

			} else {
				throw new Error("Set server names failed!");
			}
		})
		.catch((error) => {
			console.error(error);
			statusDisplay.innerText = `${error.name}: ${error.message}`;
		})
}



// main function
setChart();
selectMode(1);
