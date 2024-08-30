import "./App.css";

interface TemperatureProps {
  temp: number;
}

function LiveValue( temp : TemperatureProps) {
  if(temp.temp >= 20 && temp.temp <= 80) {
    return (
      <header className="live-value" style={{ color: "white" }}>
        {`${temp.temp.toPrecision(3)}°C`}
      </header>
    );
  } else {
    return (
      <header className="live-value" style={{ color: "red" }}>
        {`${temp.temp.toPrecision(3)}°C`}
      </header>
    );
  }

}

export default LiveValue;
