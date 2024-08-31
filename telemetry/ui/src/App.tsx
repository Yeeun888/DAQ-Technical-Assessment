import { useState, useEffect } from "react";
import LiveValue from "./live_value";
import RedbackLogo from "./redback_logo.jpg";
import "./App.css";
import useWebSocket, { ReadyState } from "react-use-websocket";
import { LineChart } from "@mui/x-charts";

const WS_URL = "ws://localhost:8080";

interface VehicleData {
  battery_temperature: number;
  timestamp: number;
}

function App() {
  // const [colorX, setColorX] = React.useState<
  //   'None' | 'piecewise' | 'continuous' | 'ordinal'
  // >('None');
  // const [colorY, setColorY] = React.useState<'None' | 'piecewise' | 'continuous'>(
  //   'piecewise',
  // );

  const [temperature, setTemperature] = useState<number>(0);
  const [tempList, setTempList] = useState<number[]>([]);
  const [counter, setCounter] = useState<number>(0);
  const [counterArray, setCounterArray] = useState<number[]>([]);

  const {
    lastJsonMessage,
    readyState,
  }: { lastJsonMessage: VehicleData | null; readyState: ReadyState } =
    useWebSocket(WS_URL, {
      share: false,
      shouldReconnect: () => true,
    });

  useEffect(() => {
    switch (readyState) {
      case ReadyState.OPEN:
        console.log("Connected to streaming service");
        break;
      case ReadyState.CLOSED:
        console.log("Disconnected from streaming service");
        break;
      default:
        break;
    }
  }, [readyState]);

  useEffect(() => {
    console.log("Received: ", lastJsonMessage);
    if (lastJsonMessage === null) {
      return;
    }

    setTemperature(lastJsonMessage["battery_temperature"]);
    
  }, [lastJsonMessage]);

  useEffect(() => {
    if (lastJsonMessage === null) {
      return;
    }
  
    if(temperature === lastJsonMessage["battery_temperature"]) {
      return;
    }
    
    if(tempList.length > 20) {
      setTempList(tempList.slice(1));
      setTempList((tempList as number[]).concat([lastJsonMessage["battery_temperature"]] ));
    } else {
      setTempList((tempList as number[]).concat([lastJsonMessage["battery_temperature"]] ));
    }

    if(counterArray.length > 20) {
      setCounterArray(counterArray.slice(1));
      setCounterArray(counterArray.concat([counter]));
    } else {
      setCounterArray(counterArray.concat([counter]));
    }

    setCounter(counter + 1);

  }, [lastJsonMessage, tempList, counterArray, counter, temperature]);

  return (
    <div className="App">
      <header className="App-header">
        <div className="nest-div">
          <div className="left-box">
            <img
              src={RedbackLogo}
              className="redback-logo"
              alt="Redback Racing Logo"
            />
            <p className="value-title">Live Battery Temperature</p>
            <LiveValue temp={temperature} />
          </div>
          <div className="right-box">
          <h4 className="relative">Temperature Since Starting Telemetry</h4>
          <LineChart
            className="line-chart"
            xAxis={[{ 
              data: counterArray,
            }]}
            series={[
              {
                data: tempList,
                showMark: ({ value }) => (value < 20 || value > 80),
                color: "white"
              }
            ]}
            yAxis={[
              {
                data: tempList
              }
            ]}
            width={600}
            height={500}
            sx= {{
              //change left yAxis label styles
              "& .MuiChartsAxis-left .MuiChartsAxis-tickLabel":{
                fill:"#FFFFFF"
              },
              // change bottom label styles
              "& .MuiChartsAxis-bottom .MuiChartsAxis-tickLabel":{
                fill:"#FFFFFF"
              },
              // bottomAxis Line Styles
              "& .MuiChartsAxis-bottom .MuiChartsAxis-line":{
              stroke:"#FFFFFF",
              strokeWidth:0.4
              },
              // leftAxis Line Styles
              "& .MuiChartsAxis-left .MuiChartsAxis-line":{
              stroke:"#FFFFFF",
              strokeWidth:0.4
              },
            }}
            grid={{ vertical: true, horizontal: true}}
          />
          <h6 className="notes">*Dangerous temperatures are hilighted with dots on the chart</h6>
          <h6 className="notes">*Reload Page to refresh data</h6>
          </div>
        </div>
      </header>
    </div>
  );
}

export default App;
