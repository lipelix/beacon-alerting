import express from "express";
import audit from "express-requests-logger";
import bodyParser from "body-parser";

const app = express();
const port = 3000;

app.use(audit({}))
app.use(bodyParser.json())

app.post("*", (req, res) => {
  console.log('Request body', JSON.stringify(req.body, null, 2));
  res.json({ status: "oks" });
});

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`);
});
