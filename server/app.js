var http = require('http');
var url = require("url");
var exec = require('child_process').exec;
var spawn = require('child_process').spawn;

var server = http.createServer(function (request, response) {
  var pathname = url.parse(request.url).pathname;

  switch (pathname) {
    case '/start':
      console.log('start');
      var a = spawn('bash', ['/home/pi/server/start.sh'], {detached: true});
      a.on('exit', function (code) {
        response.writeHead(200, {"Content-Type": "text/plain"});
        response.end();
      });
      break;
    case '/stop':
      console.log('stop');
      exec("bash /home/pi/server/stop.sh", runCallback);
      break;
    case '/status':
      console.log('status');
      exec("bash /home/pi/server/status.sh", statusCallback);
      break;
    default:
      response.writeHead(404, {"Content-Type": "text/html"});
      response.write('Not found');
      response.end();
  }

  function runCallback(error, stdout, stderr) {
    if (error) {
      response.writeHead(500, {"Content-Type": "text/html"});
      response.write('' + error);
      response.end();
      return;
    }

    response.writeHead(200, {"Content-Type": "text/plain"});
    response.end();
  }

  function statusCallback(error, stdout, stderr) {
    if (error) {
      response.writeHead(500, {"Content-Type": "text/html"});
      response.write('' + error);
      response.end();
      return;
    }

    var result = 500;
    if (stdout == 'false') {
      result = 201;
    } else if (stdout == 'true') {
      result = 202;
    }

    response.writeHead(result, {"Content-Type": "text/html"});
    response.end();
  }

});

server.listen(8000);

console.log("Server running at http://127.0.0.1:8000/");
