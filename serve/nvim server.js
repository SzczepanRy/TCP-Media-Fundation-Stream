const http = require("http");
const fs = require("fs");
const path = require("path");

// Create HTTP server
const server = http.createServer((req, res) => {
  const filePath = path.join(__dirname, "index.html"); // your HTML file

  fs.readFile(filePath, (err, content) => {
    if (err) {
      res.writeHead(500, { "Content-Type": "text/plain" });
      res.end("Error loading index.html");
      return;
    }

    res.writeHead(200, { "Content-Type": "text/html" });
    res.end(content, "utf-8");
  });
});

// Start server on port 9000
server.listen("192.168.1.100",9000, () => {
  console.log("Server running at http://localhost:9000");
});
