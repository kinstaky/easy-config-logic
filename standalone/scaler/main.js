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

const statusDisplay = document.getElementById('status');

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


		statusDisplay.innerText = 'Select realtime';
	}
}


// function fetchHistoryData(file) {
// 	const reader = new FileReader();
// 	reader.onload = () => {
// 		let jsonData = JSON.parse(reader.result);
// 		// convert seconds to time string
// 		let time = [];
// 		jsonData.seconds.forEach((element) => {
// 			const calculateTime = new Date(element*1000);
// 			// calculateTime.setTime(element*1000);
// 			let hours = calculateTime.getHours();
// 			let minutes = calculateTime.getMinutes();
// 			let seconds = calculateTime.getSeconds();
// 			time.push(`${hours}:${minutes}:${seconds}`);
// 		});
// 		// set chart
// 		var updateOption = {
// 			xAxis: {
// 				data: time
// 			},
// 			series: []
// 		};
// 		for (var i = 0; i < scalerNum; i++) {
// 			updateOption.series.push({
// 				name: `scaler ${i}`,
// 				data: jsonData.scalers[i]
// 			});
// 		}
// 		myChart.setOption(updateOption);
// 		myChart.hideLoading();
// 	}
// 	reader.readAsText(file);
// }


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
				name: `scaler ${i}`,
				type: 'line',
				symbol: 'none',
				sampling: 'lttb',
				data: []
			}
		)
		option.legend.data.push(`scaler ${i}`);
		option.legend.selected[`scaler ${i}`] = false;
	}

	// Display the chart using the configuration items and data just specified.
	myChart.setOption(option);
}


let chartTime = [];
let chartScalers = [];

function fetchHistoryData() {
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

function setHistoryChart() {
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
					name: `scaler ${i}`,
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

function setRealtimeChart() {
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
				realtimeIntervalId = setInterval(updateRealtimeChart, 1000);
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


function updateRealtimeChart() {
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
						name: `scaler ${i}`,
						data: chartScalers[i]
					});
				}

				// update chart
				myChart.setOption(updateOption);

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



// main function
setChart();
selectMode(0);
