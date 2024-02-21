import express from "express";
import bodyParser from "body-parser";

const app = express();
const port = 3000;

app.use(bodyParser.json())

app.get("*", async(req, res) => {
  res.sendFile('/Users/liborvachal/Work/beacon-alerting/playground-app/index.html');
});

app.listen(port, async() => {
  console.log(`Example app listening on http://localhost:${port}`);
});
