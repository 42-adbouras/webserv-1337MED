#!/home/mait-taj/.nvm/versions/node/v24.11.0/bin/node

/* eslint-disable no-console */
const { env, stdin, stdout } = process;

// CGI header (recommended)
// stdout.write("Content-Type: text/plain\r\n\r\n");

console.log("______________________ hello.js ______________________\n");

// Print env vars
Object.keys(env).sort().forEach((k) => {
  console.log(`${k}=${env[k]}`);
  console.log();
});

// Read request body from stdin using CONTENT_LENGTH
const clRaw = env.CONTENT_LENGTH || "0";
let contentLength = parseInt(clRaw, 10);
if (!Number.isFinite(contentLength) || contentLength < 0) contentLength = 0;

function finish(bodyBuf) {
  console.log("______________________ BODY (raw) ______________________");
  const bodyStr = bodyBuf.toString("utf8");
  console.log(bodyStr);

  const ct = env.CONTENT_TYPE || "";
  if (ct.startsWith("application/x-www-form-urlencoded")) {
    const { parse } = require("querystring");
    console.log("\n______________________ BODY (form parsed) ______________________");
    console.log(parse(bodyStr));
  }
}

let done = false;
function finishOnce(buf) {
  if (done) return;
  done = true;
  finish(buf);
}

if (contentLength === 0) {
  finishOnce(Buffer.alloc(0));
} else {
  stdin.resume();
  const chunks = [];
  let received = 0;

  stdin.on("data", (chunk) => {
    if (done) return;
    chunks.push(chunk);
    received += chunk.length;
    if (received >= contentLength) {
      stdin.pause();
      const buf = Buffer.concat(chunks, received).subarray(0, contentLength);
      finishOnce(buf);
    }
  });

  stdin.on("end", () => {
    if (done) return;
    const buf = Buffer.concat(chunks, received).subarray(0, contentLength);
    finishOnce(buf);
  });
}