const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>自動澆水控制台</title>
    <style>
        .display-flex {
            display: flex;
            flex-wrap: wrap;
        }

        #water {
            margin: 0px 15px;
            width: 240px;
            height: 390px;
            border-radius: 0 0 20px 20px;
            border: 3px solid black;
            border-top: none;
            position: relative;
            background-color: skyblue;
            overflow: hidden;
            text-align: center;
        }

        #water::before,
        #water::after {
            content: '';
            position: absolute;
            width: 450px;
            height: 430px;
            border-radius: 55% 45%;
            bottom: var(--after-bottom, 0%);
            left: 50%;
            transform: translateX(-50%);
            background-color: #fff;
            animation: rotate 6s linear infinite;
        }

        #water::after {
            left: 47%;
            bottom: var(--after-bottom, 0%);
            border-radius: 45% 50%;
            opacity: 0.3;
        }

        @keyframes rotate {
            0% {
                transform: translate(-50%) rotateZ(0deg);
            }

            100% {
                transform: translate(-50%) rotateZ(360deg);
            }
        }

        #waterValP {
            position: relative;
            color: rgb(11, 44, 155);
            font-size: 48px;
            font-family: 'fangsong';
            font-weight: bold;
            line-height: 4;
            display: inline-block;
            vertical-align: middle;
        }

        #waterRawAdc {
            position: relative;
            color: rgb(11, 155, 11);
            font-size: 36px;
            font-family: 'fangsong';
            font-weight: bold;
            line-height: 0;
            display: inline-block;
            vertical-align: middle;
        }

        input[type=text] {
            width: 78px;
            padding: 10px 15px;
            margin: 8px 5px;
            display: inline-block;
            border: 2px solid #ccc;
            border-radius: 4px;
        }

        .comboBox {
            width: 85px;
            padding: 10px 15px;
            margin: 8px 5px;
            display: inline-block;
            border: 2px solid #ccc;
            border-radius: 4px;
        }

        .button {
            background-color: #4CAF50;
            /* Green */
            border: none;
            color: white;
            padding: 9px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            cursor: pointer;
            border-radius: 12px;
            width: 120px;
        }

        .mode {
            background-color: #a89d01;
            margin: 8px 0px;
            width: 80px;
        }

        .save {
            background-color: #4c4eaf;
            margin: 8px 0px;
            width: 80px;
        }

        .open {
            background-color: #4CAF50;
            margin: 8px 0px;
        }

        .close {
            background-color: #af534c;
        }

        .label {
            color: white;
            padding: 11px;
        }

        .high_val {
            background-color: #04AA6D;
        }

        .low_val {
            background-color: #2196F3;
        }

        .lowLimit {
            background-color: #ff9800;
        }

        .interval {
            background-color: #f44336;
        }

        .water_sec {
            background-color: #e7e7e7;
            color: black;
        }

        /* Blue */

        /* Green */
    </style>
    <script>
        function init() {
            GetSetVal();
        };
        function SetBumpON() {
            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("GET", "/bumpOn");
            xmlHttp.send(null);
            xmlHttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                }
            };
        }
        function SetBumpOFF() {
            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("GET", "/bumpOff");
            xmlHttp.send(null);
            xmlHttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                }
            };
        }
        function SetMode(mode) {
            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/mode");
            xmlHttp.send(document.getElementById("mode").value);
            xmlHttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    GetSetVal();
                }
            };
        }
        function GetWaterVal() {
            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("GET", "/waterVal");
            xmlHttp.send(null);
            xmlHttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var jsonData = JSON.parse(this.responseText);
                    document.getElementById("waterValP").innerHTML = jsonData.valP + "%";
                    document.getElementById("waterRawAdc").innerHTML = jsonData.raw;
                    const element = document.querySelector("#water");
                    element.style.setProperty("--after-bottom", jsonData.valP + "%");
                }
            };
        }
        function GetSetVal() {
            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("GET", "/setVal");
            xmlHttp.send(null);
            xmlHttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var jsonData = JSON.parse(this.responseText);
                    document.getElementById("mode").selectedIndex = jsonData.mode;
                    document.getElementById("high_val").value = jsonData.highVal;
                    document.getElementById("low_val").value = jsonData.lowVal;
                    document.getElementById("lowLimit").value = jsonData.lowLimit;
                    document.getElementById("interval").value = jsonData.interval;
                    document.getElementById("water_sec").value = jsonData.waterSec;
                    setItem(jsonData.mode);
                }
            };
        }
        function changeMode(value) {
            SetMode(value);
        }
        function setItem(value) {
            if (value == 0) {
                document.getElementsByClassName("AutoMode")[0].style.display = "block";
                document.getElementsByClassName("ManualMode")[0].style.display = "none";
                document.getElementsByClassName("TimerMode")[0].style.display = "none";
            } else if (value == 1) {
                document.getElementsByClassName("AutoMode")[0].style.display = "none";
                document.getElementsByClassName("ManualMode")[0].style.display = "block";
                document.getElementsByClassName("TimerMode")[0].style.display = "none";
            } else if (value == 2) {
                document.getElementsByClassName("AutoMode")[0].style.display = "none";
                document.getElementsByClassName("ManualMode")[0].style.display = "none";
                document.getElementsByClassName("TimerMode")[0].style.display = "block";
            }
        }
        function Save(value) {
            var jsonData = {
                "highVal": document.getElementById("high_val").value,
                "lowVal": document.getElementById("low_val").value,
                "lowLimit": document.getElementById("lowLimit").value,
                "interval": document.getElementById("interval").value,
                "waterSec": document.getElementById("water_sec").value,
            }
            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/save");
            xmlHttp.send(JSON.stringify(jsonData));
            document.getElementById("high_val").value = "";
            document.getElementById("low_val").value = "";
            document.getElementById("lowLimit").value = "";
            document.getElementById("interval").value = "";
            document.getElementById("water_sec").value = "";
            xmlHttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    GetSetVal();
                }
            };
        }
        setInterval(function () {
            GetWaterVal();
        }, 2000);
    </script>
</head>

<body onload="init();">
    <div class="display-flex">
        <div id="water">
            <p id="waterValP">0%</p><br>
            <p id="waterRawAdc">0</p>
        </div>
        <div id="set">
            <div class="custom-select" style="width:400px;">
                <span class="label mode">模式</span>
                <select id="mode" class="comboBox" onchange="changeMode(this.selectedIndex)">
                    <option value="0">自動</option>
                    <option value="1">手動</option>
                    <option value="2">定時</option>
                </select>
                <button type="button" class="button save" onclick="Save()">存檔</button><br>
            </div>
            <span class="label high_val">最高水量數值</span>
            <input id="high_val" type="text">
            <Label>Raw ADC</Label><br>
            <span class="label low_val">最低水量數值</span>
            <input id="low_val" type="text">
            <Label>Raw ADC</Label>
            <div class="AutoMode">
                <span class="label lowLimit">自動澆水下限</span>
                <input id="lowLimit" type="text">
                <Label>百分比</Label>
            </div>
            <div class="ManualMode">
                <button type="button" class="button open" onclick="SetBumpON()">手動澆水</button><br>
                <button type="button" class="button close" onclick="SetBumpOFF()">停止澆水</button>
            </div>
            <div class="TimerMode">
                <span class="label interval">定時澆水間隔</span>
                <input id="interval" type="text">
                <Label>分鐘</Label><br>
                <span class="label water_sec">定時澆水秒數</span>
                <input id="water_sec" type="text">
                <Label>秒</Label><br>
            </div>
        </div>
    </div>
</body>

</html>
)=====";