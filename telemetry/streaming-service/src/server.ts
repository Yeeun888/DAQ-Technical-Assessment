import net from "net";
import { WebSocket, WebSocketServer } from "ws";

import { VDDeque } from "./deque";

export interface VehicleData {
  battery_temperature: number;
  timestamp: number;
}

const TCP_PORT = 12000;
const WS_PORT = 8080;
const tcpServer = net.createServer();
const websocketServer = new WebSocketServer({ port: WS_PORT });

tcpServer.on("connection", (socket) => {
  console.log("TCP client connected");

  var tempeartureTracker = new VDDeque();

  socket.on("data", (msg) => {
    console.log(`Received: ${msg.toString()}`);

    //Data received validation
    var jsonData: VehicleData;
    var client_message: string;
    try {
      jsonData = JSON.parse(msg.toString());
      client_message = msg.toString();
    } catch (SyntaxError) {
      console.log("Error handled, extra } removed");
      jsonData = JSON.parse(msg.toString().substring(0, msg.length - 1))
      client_message = msg.toString().substring(0, msg.length - 1);
    }

    tempeartureTracker.addNewData(jsonData);
    if(tempeartureTracker.lastFiveSecondUnsafeTemperature() > 3) {
      console.log("(%d) Unsafe Battery Limit Reached", tempeartureTracker.lastFiveSecondUnsafeTemperature())
    }

    // tempeartureTracker.DEBUGtemperature();
    // console.log("Count Errors: %d", tempeartureTracker.lastFiveSecondUnsafeTemperature());

    // Send JSON over WS to frontend clients
    websocketServer.clients.forEach(function each(client) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(client_message);
      }
    });
  });

  socket.on("end", () => {
    console.log("Closing connection with the TCP client");
  });

  socket.on("error", (err) => {
    console.log("TCP client error: ", err);
  });
});

websocketServer.on("listening", () =>
  console.log(`Websocket server started on port ${WS_PORT}`)
);

websocketServer.on("connection", async (ws: WebSocket) => {
  console.log("Frontend websocket client connected");
  ws.on("error", console.error);
});

tcpServer.listen(TCP_PORT, () => {
  console.log(`TCP server listening on port ${TCP_PORT}`);
});
