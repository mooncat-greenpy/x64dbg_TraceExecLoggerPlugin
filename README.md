# TraceExecLogger
TraceExecLogger saves information when debugging.
Logs are stored in JSON format.


## Features
|                     | Log                              | Proc Log                                                     |
| ------------------- | -------------------------------- | ------------------------------------------------------------ |
| Information to save | instruction<br>register<br>stack | module<br>thread<br>memory<br>handle<br>network              |
| Trigger             | step<br>trace<br>breakpoint      | save log<br>create thread<br>load dll<br>`TElogger.proc.log` |


## Usage
1. Download the release
2. Copy x64dbg_TraceExecLoggerPlugin.dp32/x64dbg_TraceExecLoggerPlugin.dp64 files to plugins directories of x64dbg
3. `TElogger.enable`
4. `TElogger.proc.enable`
5. Debug
6. `TElogger.save`
