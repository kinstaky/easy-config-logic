# Change log

## 2.3.0
### New features
+ modern GUI powered by flutter
+ multiple downscale expression in one line
+ user defined variables in logic expression
+ number literal (0 and 1) in logic expression
+ config backup and scaler names backup

### Optimization
+ rewrite bitsteram of FPGA
+ better scaler in FPGA


## 2.2.0
### New features
+ read scaler names from file when using web page visual mode
+ change scaler names in web page visual mode
### Optimization
+ rewrite scaler in FPGA

## 2.1.3
+ Add displaying scaler value in webpage in realtime mode below the realtime chart.
+ Add change log.
+ Change .gitignore to normal form.

## 2.1.2
### New features
+ Permits scaler server run without server(Just print scaler value like scaler).
### Optimization
+ Remove two programs: scaler and scaler_http_server. They were merged into scaler client or scaler server. So now just run one program in one system.
+ Forbids running scaler server without arguments.

## 2.1.1
### New features
+ Add http server for scaler visualization to scaler client. No need to open 2 programs now, just run ./scaler_client -s to replace ./scaler_http_server.
+ Add scaler value display in scaler client, use ./scaler_client -p to replace ./scaler_server -p or ./scaler.
### Optimization
+ Switch the scaler value display to secondary screen in scaler server(Though not recommended to display the scaler value in scaler server since the poor performance of SOC in MZTIO.)

## 2.1.0
### New features
+ Add scaler value display to scaler client.

## 2.0.3
### New features
+ Change refresh rate of the scaler value to 2Hz.
+ Switch the scaler value display to secondary screen.
+ Change scaler value to 0 when input is empty.

## 2.0.2
### Bug fix
+ Fix bugs for scaler client writing scaler value in binary files.
+ Fix bugs for scaler client falling behind 10 seconds.
+ Fix bugs for scaler server repeatly writing scaler value in the same second when the record period is 1 second.

## 2.0.1
### Bug fix
Fix bugs in using rj45 and divider.

## 2.0
The official version with all function. But actually has some bugs in rj45 output and divider. Don't use this.

## 1.9
First commit in this repository. It's not completed, just for backup.

## 1.0
Not included in this repository and it was written in C and not usable now.