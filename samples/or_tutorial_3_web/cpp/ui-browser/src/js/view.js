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
    colorEnabled: false,
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
const canvasLeft = 0;
const canvasTop = 0;
const canvasBottom = 480;
const canvasRight = 640;

let view = new Vue({
    el: '#poseview',
    data: viewModel,
    computed: {
        isConnected: function() {
            return this.status === ViewStatus.CONNECTED;
        },
    },
    watch: {
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
            } else {
                 console.log("Calling transporter close");
                 transporter.close();
             }
         },
        stop: () => {
            transporter.close();
        },
    }
});

function getHostname() {
    let saved = localStorage.getItem('host');
    return (!!saved) ? saved : "ws://"+location.host;
}
const transporter = new SpTransport(getHostname());

function Log(tag) {
    return function(arg) {
        // rec.save(tag, arg);
        console.log(tag, arguments);
    };
}

let poselist = [];

function resetAll() {
 //   omap.reset();
 //   orender.reset();
 //   poselist = [];
 //   drawPoses(poselist);
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

transporter.onEvent = (event, msg) => {
    console.log(event, msg);
    if (event === 'on_reset_completed') {
        console.log("got reset event...");
        resetAll();
    }
};

let rgb_view = new BufferViewer();
rgb_view.init(document.querySelector('#color-canvas'));
let rgbRenderCall = rgb_view.render.bind(rgb_view);

transporter.onORTrackInfo =  (timestamp, or_data) => {

        var or_result_list = or_data.Object_result;

        for (var i = 0; i < or_result_list.length; i++) {
            var or_object = or_result_list[i];
            
            var rectX = or_object.rectangle[0];
            var rectY = or_object.rectangle[1];
            var rectW = or_object.rectangle[2];
            var rectH = or_object.rectangle[3];
            var rectX2 = rectX + rectW;
            var rectY2 = rectY + rectH;
            
            var label = or_object.label;
            var currentTime = getCurrentTime(timestamp);
            var data = currentTime + ";" + or_object.label +  ";"  + "(" + rectX + ","  + rectY + ")" + " (" + rectX2 + ","  + rectY2 + ")";
	    console.log(data);
	    insertDataInTableRows(data);

	}
}; 

transporter.onColorFrame = (timestamp, width, height, data, message) => {
    // Draw the frame into the canvas:
    rgb_view.updateBuffer(data, width, height, THREE.RGBFormat);
    requestAnimationFrame(rgbRenderCall);
    
    // Get the color-2d-overlay canvas that is overlaid on top of the RGB canvas:
    var canvas2d = document.getElementById("color-2d-overlay");
    if (canvas2d == null) {
        console.log("Failed to get color-2d-overlay element");
        return 0;
    } 
    var ctx2d = canvas2d.getContext("2d");
    if (ctx2d == null)  {
        console.log("Failed to get 2d context from color-2d-overlay element");
        return 0;
    } 

    ctx2d.clearRect(0, 0, canvas2d.width, canvas2d.height); // Clear the overlay canvas
    
    // Now, if we have any recognition data, draw onto the overlay canvas using 2d API:
    if (message === undefined) {
         return 0;
    } else {
        ctx2d.lineWidth = 2;
        ctx2d.font = "11px sans-serif";
        ctx2d.fillStyle = "rgba(250, 50, 50, 1.0)";     // Color of text
        ctx2d.strokeStyle = "rgba(250, 50, 50, 1.0)";       
        ctx2d.shadowColor = "rgba(0, 0, 0, 0.5)";
        ctx2d.shadowOffsetX = 2;
        ctx2d.shadowOffsetY = 3;
        ctx2d.shadowBlur = 4;
    
        var or_result_list = message.Object_result;

        // For each entry in the object recognition array:
        for (var i = 0; i < or_result_list.length; i++) {
            var or_object = or_result_list[i];
            
            var rectX = or_object.rectangle[0];
            var rectY = or_object.rectangle[1];
            var rectW = or_object.rectangle[2];
            var rectH = or_object.rectangle[3];
            
           // var confidence = Math.floor(or_object.confidence*100);
            var label = or_object.label;
	    var intersect = isIntersect(rectX, rectY, rectW, rectH);
	    console.log("Canvas and frame intersect: " + intersect + " for " + rectX + "," + rectY + "," + rectW + "," + rectH);
          // Draw rect with label if the co-ordinates are anywhere in the canvas.
            if(isIntersect(rectX, rectY, rectW, rectH)) {
                console.log("Intersects. Drawing rectangle and labeling");
                ctx2d.strokeRect(rectX, rectY, rectW, rectH);  
                ctx2d.fillText(label, rectX+1, rectY+10);
            }

        }       
    }
};

transporter.removeLoader = () => {
    var loader = document.getElementById("modal");
    if (loader) {
         console.log("Loader exists. Deleting it.");
         loader.parentNode.removeChild(loader);
         if(!document.getElementById("modal")) {
                this.loaderRemoved=true;
         } 
    }
};

transporter.onORLabel = (or_label) => {
    var para = document.createElement("p");
    var str = "";

    var or_label_list = or_label.list;
    for (var i = 0; i < or_label_list.length; i++) {
        str = str + or_label_list[i];
        if(i != or_label_list.length-1) {
            str = str + ", ";
        }
    }
    console.log("or label list is: ", str);

    var node = document.createTextNode(str);
    para.appendChild(node);
    var element = document.getElementById("labelList");
    element.appendChild(para);
}

function insertDataInTableRows(data) {
    var entry = data.split(";");
    var table = document.getElementById("tBody");
    var currTableRows = table.rows.length;
    
    if (currTableRows === maxTableRows) {
        lastRow = table.rows.length - 1;             // set the last row index
 
        table.deleteRow(lastRow);
    } 
    var row = table.insertRow(0); 
    var arrayLength = entry.length;
    for (var count = 0; count < arrayLength; count++) {
        fillCell(row.insertCell(count), entry[count], 'row');
    }       

}

// Create <div> element and append to the table cell
function fillCell(cell, text, style) {
    var div = document.createElement('div'), // create DIV element
        txt = document.createTextNode(text); // create text node
    div.appendChild(txt);                    // append text node to the DIV
    div.setAttribute('class', style);        // set DIV class attribute
    div.setAttribute('className', style);    // set DIV class attribute for IE (?!)
    cell.appendChild(div);                   // append DIV to the table cell
}

function isIntersect(frameLeft, frameTop, frameWidth, frameHeight){
       var frameRight = frameLeft + frameWidth;
       var frameBottom = frameTop + frameHeight;

       if (frameLeft < canvasRight && frameRight > canvasLeft &&
            frameTop < canvasBottom && frameBottom > canvasTop ) {
	return true;
       } else { 
        return false;
       }
}

function zeroPadding(time,numberOfZeros) {
    while (time.toString().length < numberOfZeros) {
        time = "0" + time;
    }
    return time;
}

function getCurrentTime(date) {
    var h = zeroPadding(date.getHours(), 2);
    var m = zeroPadding(date.getMinutes(), 2);
    var s = zeroPadding(date.getSeconds(), 2);
    var ms = zeroPadding(date.getMilliseconds(), 3);
    return(h + ":" + m + ":" + s + ":" + ms);
}

setTimeout(function() {
    transporter.open();
}, 100);
