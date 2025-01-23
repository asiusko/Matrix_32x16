const char* wifiJoystickPage = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tetris Controls</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: space-between;
            align-items: center;
            height: 100%;
            width: 100%;
            background: linear-gradient(to bottom, #9fb9d5, #d2e0c3);
        }

        .container {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100%;
            width: 50%;
        }

        .container-separator {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100%;
            width: 10%;
        }

        .control-arrows {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 10px;
        }

        .control-arrows .blank {
            visibility: hidden;
        }

        .control-btn {
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            gap: 20px;
        }

        .round-btn {
            width: 100px;
            height: 100px;
            background-color: #808080;
            color: white;
            border: none;
            border-radius: 50%;
            box-shadow: inset -2px -2px 4px rgba(0, 0, 0, 0.2), inset 2px 2px 4px rgba(255, 255, 255, 0.3);
            font-size: 3em;
            font-weight: bold;
            cursor: pointer;
        }

        .round-btn:hover {
            background-color: #707070;
        }

        .oval-btn {
            width: 150%;
            height: 150%;
            background-color: #808080;
            color: white;
            border: none;
            border-radius: 40%;
            box-shadow: inset -2px -2px 4px rgba(0, 0, 0, 0.2), inset 2px 2px 4px rgba(255, 255, 255, 0.3);
            font-size: 16px;
            font-weight: bold;
            letter-spacing: 10px;
            cursor: pointer;
            padding: 10px;
        }

        .oval-btn:hover {
            background-color: #707070;
        }
    </style>
    <script>
        async function sendGet(url, action) {
            await fetch(url+"?action="+action, {
                method: 'GET'
            });
        }
    </script>
</head>
<body>
<div class="container-separator" style="justify-content: flex-start"></div>
<div class="container" style="justify-content: flex-end;">
    <div class="control-arrows">
        <div class="blank"></div>
        <button class="round-btn" onclick="sendGet('control', 'up') ">▲</button>
        <div class="blank"></div>
        <button class="round-btn" onclick="sendGet('control', 'left') ">◀</button>
        <div class="blank"></div>
        <button class="round-btn" onclick="sendGet('control', 'right') ">▶</button>
        <div class="blank"></div>
        <button class="round-btn" onclick="sendGet('control', 'down') ">▼</button>
        <div class="blank"></div>
    </div>
</div>
<div class="container-separator" style="justify-content: flex-start"></div>
<div class="container-separator" style="justify-content: flex-start"></div>
<div class="container-separator" style="justify-content: flex-start"></div>
<div class="container" style="justify-content: flex-start;">
    <div class="control-btn">
        <br>
        <button class="oval-btn"  onclick="sendGet('control', 'select') ">SELECT</button>
        <button class="oval-btn"  onclick="sendGet('control', 'start') ">START</button>
        <button class="round-btn" onclick="sendGet('control', 'btnA') ">A</button>
        <button class="round-btn" onclick="sendGet('control', 'btnB') ">B</button>
        <br>
    </div>
</div>
</body>
</html>
)";
