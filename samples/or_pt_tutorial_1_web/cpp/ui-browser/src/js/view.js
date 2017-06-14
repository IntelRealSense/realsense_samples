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

this.prevPid=-1;
transporter.onPTDataUpdate = (timestamp, pt_data) => {
    if (pt_data === undefined) {
        console.log("The PT data is not available");
    } else {
        var scaleFactor = 1;        

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
            var data_final = getCurrentTime(timestamp) + ";" + pid +";";

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

                if(pt_object.hasOwnProperty('pid'))
                {
                    ctx2d.fillText("pid: " + pt_object.pid, rectX+1, rectY+10);
                }

            }
            
            if(pt_object.hasOwnProperty('center_mass_image'))
            {
                ctx2d.strokeRect(pt_object.center_mass_image.x * scaleFactor, pt_object.center_mass_image.y * scaleFactor, 1, 1);    // Draw center of mass image
            }

        }        
 
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
            
            var confidence = Math.floor(or_object.confidence*100);
            var label = or_object.label;
        
            // Draw boxes over recognized objects in the image:
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
            
            ctx2d.strokeRect(rectX, rectY, rectW, rectH);   // Draw rect with label
            ctx2d.fillText(label + " (" + confidence + ")", rectX+1, rectY+10);
        }       
    }
};

transporter.onORInfo = (timestamp, or_data) => {

        var or_result_list = or_data.Object_result;
        for (var i = 0; i < or_result_list.length; i++) {
            var or_object = or_result_list[i];

            var x = or_object.centerCoord[0].toFixed(2);
            var y = or_object.centerCoord[1].toFixed(2);
            var z = or_object.centerCoord[2].toFixed(2);
            var confidence = (or_object.confidence*100).toFixed(2);

            var rectX = or_object.rectangle[0];
            var rectY = or_object.rectangle[1];
            var rectW = or_object.rectangle[2];
            var rectH = or_object.rectangle[3];
            
            var currentTime = getCurrentTime(timestamp);

            
            var data = currentTime + ";" + or_object.label +  ";" + x + ", " + y + ", " + z + ";" + confidence;
            
            insertDataInTableRows(data);
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
