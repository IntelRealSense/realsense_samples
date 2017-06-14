// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
const ViewStatus = {
    DISCONNECTED: 0,
    CONNECTING: 1,
    CONNECTED: 2
};

let viewModel = {
    S: ViewStatus,
    status: ViewStatus.DISCONNECTED,
    fisheyeEnabled: false,
    pose: {x:0, y:0, z:0},
    tracking: -1,
    wsurl: ''
};

const TrackingStatus = [
    {code: 0, text: "fail"},
    {code: 1, text: "low"},
    {code: 2, text: "medium"},
    {code: 3, text: "high"},
    {code: 4, text: "reset"}
];

const maxTableRows = 50;

let view = new Vue({
    el: '#poseview',
    data: viewModel,
    computed: {
        isConnected: function() {
            return this.status === ViewStatus.CONNECTED;
        },
        trackingText: function() {
            return this.tracking >= 0 ? TrackingStatus[this.tracking].text : "na";
        }
    },
    watch: {
        'selected': function (val, oldVal) {
            console.log(val);
            if (val === "top") {
                mapview.camera.position.set(0, 16, 0.5);
            } else {
                mapview.camera.position.set(16, 0, 0);
            }
            //   console.log('new: %s, old: %s', val, oldVal)
        },
        'fisheyeEnabled': function (val) {
            console.log("subscribing to fisheye: " + val);
            transporter.sendMessage({
                type: "control",
                command: "fisheye", subscribe: !!val
            });
        },
        'colorEnabled': function (val) {
            console.log("subscribing to color: " + val);
            transporter.sendMessage({
                type: "control",
                command: "color", subscribe: !!val
            });
        }
    },
    methods: {
        check: () => {
            if (document.getElementById("tButtonName").innerText == "Resume")
            {
                console.log("Calling transporter open");
                transporter.open();
            }
            else
            {
                console.log("Calling transporter close");
                transporter.close();
            }
        },
        stop: () => {
            transporter.close();
        },
        reset: () => {
            viewModel.tracking = 4;
            //transporter.sendMessage({ type: "control", command: "reset" });
            transporter.sendMessage({ "type": "control", "command": "reset" });
        }
    }
});

function getHostname() {
    let saved = localStorage.getItem('host');
    return (!!saved) ? saved : "ws://"+location.host;
}
const transporter = new SpTransport(getHostname());

// 64 x 64 tiles >> 1 tile == 128 x 128 pixels >> 1 pixel == 0.05m x 0.05m (see slam->get)
// == 409.6 x 409.6 meters
const omap = new OccupancyMap(64, 64, 128);
const orender = new OccupancyMapRenderer(omap, mapview.scene, 100);

function populate_test() {
    for (let i = -100; i < 100; i++) {
        omap.set(Math.sin(i/20)*20, i, 40 + (i%20));
        omap.set(Math.sin(i/20)*20, i+1, 20 + (i%20));

        orender.update();
    }
}

let rec = new EventRecorder();
// function saveToLocalStorage() {
//     localStorage.setItem('pose', JSON.stringify(rec.events.get("pose").data));
// }

// function loadPoseList() {
//    let e = JSON.parse(localStorage.getItem('pose'));
//    let i = 0;
//    let size = e.length;
//    let emitter = new Emitter(function() {
//        return e[(i++)%length];
//    }, 100);
//    emitter.e = e;
//    return emitter;
// }

function Log(tag) {
    return function(arg) {
        // rec.save(tag, arg);
        console.log(tag, arguments);
    };
}

let poselist = [];

function resetAll() {
    console.log("resetAll");
    omap.reset();
    orender.reset();
    reset_metadata();
    poselist = [];
    drawPoses(poselist);
}

transporter.onOpen = () => {
    console.log("connected");
    viewModel.wsurl = transporter._ws.url;
    viewModel.status = ViewStatus.CONNECTED;
    document.getElementById("tButtonName").innerHTML = "Pause";
};

transporter.onClose = () => {
    console.log("disconnected");
    viewModel.wsurl = '';
    viewModel.status = ViewStatus.DISCONNECTED;
    document.getElementById("tButtonName").innerHTML = "Resume";
};

transporter.onPoseUpdate = (pose) => {
    // console.log('pose', pose);
    // rec.save('pose', pose);
    viewModel.pose.x = pose.pose[3];
    viewModel.pose.y = pose.pose[7];
    viewModel.pose.z = pose.pose[11];
    viewModel.tracking = pose.tracking;
    poselist.push(pose);
    drawPoses(poselist);
};

transporter.onEvent = (event, msg) => {
    console.log(event, msg);
    if (event === 'on_reset_completed') {
        console.log("got reset event...");
        resetAll();
    }
};

transporter.onMapUpdate = (scale_mm, mapupdate) => {
    // console.log('map', mapupdate);
    // rec.save('mapupdate', update);
    orender.setScale(scale_mm);
    const len = mapupdate.length;
    let p = 0;
    while (p < len) {
        omap.set(mapupdate[p++], mapupdate[p++], mapupdate[p++]);
    }
    orender.update();
};

transporter.onFpsUpdate = (f) => {
    // "input" or "tracking"
    let stats = (f.fps.type === "input") ? statsIn : statsOut;

    if (stats.counter++ === 0) {
        //skip first set
        return;
    }
    stats.fish.update(f.fps.fisheye, 35);
    stats.depth.update(f.fps.depth, 35);
    stats.gyro.update(f.fps.gyroscope, 230);
    stats.accel.update(f.fps.accelerometer, 260);
};

transporter.removeLoader = () => {
    var loader = document.getElementById("modal");
    if (loader) {
	console.log("Loader exists. Deleting it.");
        loader.parentNode.removeChild(loader);
        if(!document.getElementById("modal")){
                 this.loaderRemoved=true;
        }
    }
};

var stats1 = new Stats();
var statsIn = {
    counter: 0,
    fish: stats1.addPanel( new Stats.Panel( 'fish', '#ff8', '#221' ) ),
    depth: stats1.addPanel( new Stats.Panel(  'depth', '#0f0', '#221' ) ),
    gyro: stats1.addPanel( new Stats.Panel( 'gyr.', '#0ff', '#002' ) ),
    accel: stats1.addPanel( new Stats.Panel( 'acc.', '#f8f', '#212') )
};
var stats2 = new Stats();
var statsOut = {
    counter: 0,
    fish: stats2.addPanel( new Stats.Panel( 'fish', '#ff8', '#221' ) ),
    depth: stats2.addPanel( new Stats.Panel( 'depth', '#0f0', '#221' ) ),
    gyro: stats2.addPanel( new Stats.Panel( 'gyr.', '#0ff', '#002' ) ),
    accel: stats2.addPanel( new Stats.Panel( 'acc.', '#f8f', '#212') )
    // fish: stats2.addPanel( new Stats.Panel( 'fish', '#f8f', '#212'  ) ),
    // gyro: stats2.addPanel( new Stats.Panel( 'gyro', '#0f0', '#020' ) )
};

document.querySelector('.stats1').appendChild( stats1.dom );
document.querySelector('.stats2').appendChild( stats2.dom );

let fisheye_view = new BufferViewer("Fisheye");
fisheye_view.init(document.querySelector('#fisheye-canvas'));
let fisheyeRenderCall = fisheye_view.render.bind(fisheye_view);

let rgb_view = new BufferViewer("color");
rgb_view.init(document.querySelector('#color-canvas'));
let rgbRenderCall = rgb_view.render.bind(rgb_view);

transporter.onFisheyeFrame = (function() {
    var last_display = 0;// = Date.now();
    return (timestamp, width, height, data) => {
        //lastFisheyeTs = timestamp;
        let now = Date.now();
        let wait = 0;
        if (now - last_display < 28) {
            wait = 32 - (now - last_display);
        }
        setTimeout(() => {
            last_display = Date.now();
            fisheye_view.updateBuffer(data, width, height, THREE.LuminanceFormat);
            requestAnimationFrame(fisheyeRenderCall);
        }, wait);
    };
})();

transporter.onORDataUpdate = (or_data) => {
    var or_result_list = or_data.Object_result;

    for (var i = 0; i < or_result_list.length; i++) {
        var or_object = or_result_list[i];

        var x = or_object.pose[0];
        var y = or_object.pose[1];
        var z = or_object.pose[2];
        drawORObjects(or_object.pose, or_object.label);
    }

};

this.prevPid=-1;
transporter.onPTDataUpdate = (timestamp, pt_data) => {
    if (pt_data === undefined) {
        console.log("The PT data is not available");
    } else {
        var scaleFactor = 0.5;

        var canvas2d=document.getElementById("color-2d-overlay");
        if (canvas2d == null) {
            console.log("Failed to get color-2d-overlay element");
        }

        var ctx2d = canvas2d.getContext("2d");
        if (ctx2d == null)    {
            console.log("Failed to get 2d context from color-2d-overlay element");
        }

        //ctx2d.clearRect(0, 0, canvas2d.width, canvas2d.height);    // Clear the overlay canvas

        ctx2d.lineWidth = 2;

        ctx2d.font = "11px sans-serif";
        ctx2d.fillStyle = "rgba(255, 255, 0, 1.0)";        // Color of text - yellow
        ctx2d.strokeStyle = "rgba(250, 50, 50, 1.0)";      // Color of stroke rectangle - red
        ctx2d.shadowColor = "rgba(0, 0, 0, 0.5)";
        ctx2d.shadowOffsetX = 2;
        ctx2d.shadowOffsetY = 3;
        ctx2d.shadowBlur = 4;

        var pt_result_list = pt_data.Object_result;

        for (var i = 0; i < pt_result_list.length; i++) {
            var pt_object = pt_result_list[i];

            var pid = pt_object.pid;
            var data_final = getCurrentTime(timestamp) + ";" + pid + ";";

            if(pt_object.hasOwnProperty('center_mass_world'))
            {
                var CenterMassWorldX = pt_object.center_mass_world.x.toFixed(4);
                var CenterMassWorldY = pt_object.center_mass_world.y.toFixed(4);
                var CenterMassWorldZ = pt_object.center_mass_world.z.toFixed(4);

                data_final += CenterMassWorldX + ", " + CenterMassWorldY + ", " + CenterMassWorldZ + ";";

                if(this.prevPid != pid)
                {
                    insertDataInTableRows(data_final);
                    this.prevPid = pid;
                    //console.log(data_final);
                }
            }

            if(pt_object.hasOwnProperty('person_bounding_box'))
            {
                var rectX = pt_object.person_bounding_box.x * scaleFactor;
                var rectY = pt_object.person_bounding_box.y * scaleFactor;
                var rectW = pt_object.person_bounding_box.w * scaleFactor;
                var rectH = pt_object.person_bounding_box.h * scaleFactor;

                ctx2d.strokeRect(rectX, rectY, rectW, rectH); //Draw each person rectangle

                ctx2d.fillText("pid: " + pid, rectX+1, rectY+10);
            }

            if(pt_object.hasOwnProperty('center_mass_image'))
            {
                ctx2d.strokeRect(pt_object.center_mass_image.x * scaleFactor, pt_object.center_mass_image.y * scaleFactor, 1, 1);    // Draw center of mass image
            }

            if(pt_object.hasOwnProperty('pose'))
            {
                var x = pt_object.pose[0];
                var y = pt_object.pose[1];
                var z = pt_object.pose[2];
                console.log("x : ", x, " y : ", y, " z : ", z);
                drawPersonData(pt_object.pose);
            }
        }

    }

}


transporter.onColorFrame = (timestamp, width, height, data, message) => {
    //lastColorTs = timestamp;

    rgb_view.updateBuffer(data, width, height, THREE.RGBFormat);
    requestAnimationFrame(rgbRenderCall);

	var canvas2d=document.getElementById("color-2d-overlay");
	if (canvas2d == null) {
		console.log("Failed to get color-2d-overlay element");
	}

	var ctx2d = canvas2d.getContext("2d");
	if (ctx2d == null)	{
		console.log("Failed to get 2d context from color-2d-overlay element");
	}

	ctx2d.clearRect(0, 0, canvas2d.width, canvas2d.height);	// Clear the overlay canvas

	if (message === undefined) {
		// console.log("The OR data is not available");
	} else {
	    ctx2d.lineWidth = 2;

		ctx2d.font = "11px sans-serif";
		ctx2d.fillStyle = "rgba(250, 50, 50, 1.0)";		// Color of text
		ctx2d.strokeStyle = "rgba(250, 50, 50, 1.0)";
		ctx2d.shadowColor = "rgba(0, 0, 0, 0.5)";
		ctx2d.shadowOffsetX = 2;
		ctx2d.shadowOffsetY = 3;
		ctx2d.shadowBlur = 4;

	    var or_result_list = message.Object_result;

	for (var i = 0; i < or_result_list.length; i++) {
		var or_object = or_result_list[i];
		// console.log("label", or_object.label);
		// console.log("confidence", or_object.confidence);

            var x = or_object.pose[0];
            var y = or_object.pose[1];
            var z = or_object.pose[2];
            // console.log("OR obj x:", x, "y:", y, "z:",z);

		var rectX = or_object.rectangle[0]/2;
		var rectY = or_object.rectangle[1]/2;
		var rectW = or_object.rectangle[2]/2;
		var rectH = or_object.rectangle[3]/2;

		var confidence = Math.floor(or_object.confidence*100);
		var label = or_object.label;

		if(confidence < 50){
				ctx2d.fillStyle = "red";
				ctx2d.strokeStyle = "red";
			} else if (confidence > 90) {
				ctx2d.strokeStyle = "green";
				ctx2d.fillStyle = "green";
			} else {
				ctx2d.fillStyle = "yellow";
				ctx2d.strokeStyle = "yellow";
			}

			ctx2d.strokeRect(rectX, rectY, rectW, rectH);	// Draw rect in 2d context
			ctx2d.fillText(label + " (" + confidence + ")", rectX+1, rectY+10);

			var data = label +  ";" + x + ", " + y + ", " + z + ";" + confidence;
			// console.log(data);

	}

	}
};

transporter.onORInfo = (timestamp, or_data) => {

	    var or_result_list = or_data.Object_result;
		console.log("Number of objects recognised is: " + or_result_list.length);
	for (var i = 0; i < or_result_list.length; i++) {
		var or_object = or_result_list[i];
		// console.log("label", or_object.label);
		// console.log("confidence", or_object.confidence);


            var x = or_object.pose[0].toFixed(3);
            var y = or_object.pose[1].toFixed(3);
            var z = or_object.pose[2].toFixed(3);
            // console.log("OR obj x:", x, "y:", y, "z:",z);

		var rectX = or_object.rectangle[0]/2;
		var rectY = or_object.rectangle[1]/2;
		var rectW = or_object.rectangle[2]/2;
		var rectH = or_object.rectangle[3]/2;
			var confidence = (or_object.confidence*100).toFixed(2);

			// console.log(or_object.label +  ";" + rectX + "; " + rectY + "; " + rectW + ";" + rectH + ";" + confidence);

			var currentTime = getCurrentTime(timestamp);
			var data = currentTime + ";" + or_object.label +  ";" + x + ", " + y + ", " + z + ";" + confidence;
			// console.log(data);
			insertDataInTableRows(data);

		}
};

function insertDataInTableRows(data) {
	var entry = data.split(";");
    var table = document.getElementById("tBody");
    var currTableRows = table.rows.length;

    if(currTableRows === maxTableRows){
	console.log("Table has max rows. Deleting one row");
	lastRow = table.rows.length - 1;             // set the last row index

	table.deleteRow(lastRow);
    }
    var row = table.insertRow(0);
    var arrayLength = entry.length;
    for (var count = 0; count < arrayLength; count++) {
        fillCell(row.insertCell(count), entry[count], 'row');
    }

}

// create <div> element and append to the table cell
function fillCell(cell, text, style) {
    var div = document.createElement('div'), // create DIV element
        txt = document.createTextNode(text); // create text node
    div.appendChild(txt);                    // append text node to the DIV
    div.setAttribute('class', style);        // set DIV class attribute
    div.setAttribute('className', style);    // set DIV class attribute for IE (?!)
    cell.appendChild(div);                   // append DIV to the table cell
}

function zeroPadding(time,numberOfZeros) {
    while (time.toString().length < numberOfZeros) {
        time = "0" + time;
    }
    return time;
}

function getCurrentTime(date) {
//    var date = new Date();
    var h = zeroPadding(date.getHours(), 2);
    var m = zeroPadding(date.getMinutes(), 2);
    var s = zeroPadding(date.getSeconds(), 2);
    var ms = zeroPadding(date.getMilliseconds(), 3);
    return(h + ":" + m + ":" + s + ":" + ms);
}

setTimeout(function() {
    transporter.open();
}, 100);
