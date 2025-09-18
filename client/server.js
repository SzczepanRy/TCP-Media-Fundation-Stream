// bridge.js
// bridge.js
const net = require('net');
const WebSocket = require('ws');

const tcpPort = 12345; // C++ client will connect here
const wsPort = 9000;

// WebSocket server for browser clients
const wss = new WebSocket.Server({ port: wsPort });
console.log(`WebSocket server listening on ws://localhost:${wsPort}`);

// serving files


// TCP server for C++ client
const tcpServer = net.createServer((socket) => {
    console.log("C++ client connected");

    let buffer = Buffer.alloc(0);

    socket.on('data', (data) => {
        buffer = Buffer.concat([buffer, data]);

        // Extract frames (4-byte length prefix)
        while (buffer.length >= 4) {
            const frameSize = buffer.readUInt32BE(0);
            if (buffer.length < 4 + frameSize) break;

            const frame = buffer.slice(4, 4 + frameSize);
            buffer = buffer.slice(4 + frameSize);

            // Forward frame to all connected WebSocket clients
            wss.clients.forEach(ws => {
                if (ws.readyState === WebSocket.OPEN) {
                    ws.send(frame);
                }
            });
        }
    });

    socket.on('close', () => {
        console.log("C++ client disconnected");
    });
});

tcpServer.listen(tcpPort, '0.0.0.0', () => {
    console.log(`TCP server listening on 127.0.0.1:${tcpPort}`);
});
