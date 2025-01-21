const char* wifiConfigPage = R"(
<!DOCTYPE html>
<html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Wi-Fi Configuration</title>
      <style>
          body {
              font-family: Arial, sans-serif;
              margin: 0;
              padding: 0;
              display: flex;
              justify-content: center;
              align-items: center;
              height: 100vh;
              background: linear-gradient(to bottom, #9fb9d5, #d2e0c3) !important;
          }

          .container {
              background-color: white;
              padding: 20px;
              border-radius: 8px;
              box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
              max-width: 400px;
              width: 100%;
          }

          h2 {
              text-align: center;
              margin-bottom: 20px;
          }

          label {
              font-size: 16px;
              margin-bottom: 5px;
              display: inline-block;
          }

          input[type="text"] {
              width: 100%;
              padding: 10px;
              margin: 10px 0;
              border: 1px solid #ccc;
              border-radius: 4px;
              box-sizing: border-box;
          }

          input[type="submit"] {
              width: 100%;
              background-color: #4CAF50;
              color: white;
              padding: 12px 20px;
              border: none;
              border-radius: 4px;
              cursor: pointer;
              font-size: 16px;
          }

          input[type="submit"]:hover {
              background-color: #45a049;
          }
      </style>
  </head>
  <body>
  <div class="container">
      <h2>Wi-Fi Configuration</h2>
      <form action="/configure" method="post">
          <label for="ssid">SSID:</label><br>
          <input type="text" id="ssid" name="ssid"><br><br>
          <label for="password">Password:</label><br>
          <input type="text" id="password" name="password"><br><br>
          <input type="submit" value="Connect">
      </form>
      <br>
      <form action="/restart" method="post">
          <input type="submit" value="Restart ESP">
      </form>
  </div>
  </body>
</html>
)";
