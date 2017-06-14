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

const maxTableRows = 40;
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

this.prevX=0.0;
this.prevY=0.0;
this.prevZ=0.0;
this.prevOrientation="";
transporter.onPTDataUpdate = (timestamp, pt_data) => {
    var tracking_result=false, head_pose_result=false, orientation_result=false; gesture_result=false;
    if (pt_data === undefined) {
        console.log("The PT data is not available");
    } else {
        var scaleFactor = 2;        

        var canvas2d=document.getElementById("color-2d-overlay");
        if (canvas2d == null) {
            console.log("Failed to get color-2d-overlay element");
        } 

        var ctx2d = canvas2d.getContext("2d");
        if (ctx2d == null)    {
            console.log("Failed to get 2d context from color-2d-overlay element");
        } 

        ctx2d.clearRect(0, 0, canvas2d.width, canvas2d.height);    // Clear the overlay canvas
        
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

            var data_final = getCurrentTime(timestamp) + ";" + pt_object.pid;

            //PT Sample 1 result
            if(pt_object.hasOwnProperty('center_mass_world'))
            {
                var CenterMassWorldX = pt_object.center_mass_world.x.toFixed(4);
                var CenterMassWorldY = pt_object.center_mass_world.y.toFixed(4);
                var CenterMassWorldZ = pt_object.center_mass_world.z.toFixed(4);
                
                data_final += ";" + CenterMassWorldX + ", " + CenterMassWorldY + ", " + CenterMassWorldZ+ ";";

                if( Math.abs(this.prevX-CenterMassWorldX)>0.3 || Math.abs(this.prevY-CenterMassWorldY)>0.3 || Math.abs(this.prevZ-CenterMassWorldZ)>0.3)//To Reduce print overflow
                {
                    this.prevX=CenterMassWorldX;
                    this.prevY=CenterMassWorldY;
                    this.prevZ=CenterMassWorldZ;
                    tracking_result = true;
                }
            }

            if(pt_object.hasOwnProperty('cumulative_total'))
            {
                data_final += pt_object.cumulative_total;
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

            //PT sample 2 result
            if(pt_object.hasOwnProperty('head_bounding_box'))
            {
                var rectX = pt_object.head_bounding_box.x * scaleFactor;
                var rectY = pt_object.head_bounding_box.y * scaleFactor;
                var rectW = pt_object.head_bounding_box.w * scaleFactor;
                var rectH = pt_object.head_bounding_box.h * scaleFactor;

                ctx2d.strokeRect(rectX, rectY, rectW, rectH); //Draw head bounding box
            }

            if(pt_object.hasOwnProperty('head_pose'))
            {
                var Pitch = pt_object.head_pose.pitch.toFixed(4);
                var Roll = pt_object.head_pose.roll.toFixed(4);
                var Yaw = pt_object.head_pose.yaw.toFixed(4);

                if( Math.abs(this.prevX-Pitch)>5.0 || Math.abs(this.prevY-Roll)>4.0 || Math.abs(this.prevZ-Yaw)>5.0)//To Reduce print overflow
                {
                    data_final += ";" + Pitch + ", " + Roll + ", " + Yaw;

                    this.prevX=Pitch;
                    this.prevY=Roll;
                    this.prevZ=Yaw;
                    head_pose_result = true;
                }
            }

            if(pt_object.hasOwnProperty('person_orientation'))
            {
                var Orientation = pt_object.person_orientation.orientation;
                var Confidence = pt_object.person_orientation.confidence;

                if(this.prevOrientation != Orientation)
                {
                    this.prevOrientation = Orientation;
                    orientation_result = true;
                }

                if(head_pose_result == true)
                {
                    data_final += ";" + Orientation + "(" + Confidence + "%)";
                }
                else
                {
                    data_final += ";" + "Not Detected" + ";" + Orientation + "(" + Confidence + "%)";
                }
            }

            //PT sample 3 result
            if(pt_object.hasOwnProperty('gesture_color_coordinates'))
            {
                var OriginX = pt_object.gesture_color_coordinates.origin.x;
                var OriginY = pt_object.gesture_color_coordinates.origin.y;
                var DirectionX = pt_object.gesture_color_coordinates.direction.x;
                var DirectionY = pt_object.gesture_color_coordinates.direction.y;
                
                var FromX = OriginX * scaleFactor;
                var FromY = OriginY * scaleFactor;
                var ToX = (OriginX + 400 * DirectionX) * scaleFactor;
                var ToY = (OriginY + 400 * DirectionY) * scaleFactor;

                //console.log("Origin - Direction XY: " + OriginX + ", " + OriginY + ", " + DirectionX + ", " + DirectionY);
                //console.log("From - t0 XY: " + FromX + ", " + FromY + ", " + ToX + ", " + ToY);
                var headlen = 10;   // length of head in pixels
                var angle = Math.atan2(ToY-FromY,ToX-FromX);

                ctx2d.strokeStyle = "rgba(0, 238, 0, 1.0)";      // Color of pointing - green
                ctx2d.beginPath();
                ctx2d.moveTo(FromX, FromY);
                ctx2d.lineTo(ToX, ToY);
                ctx2d.lineTo(ToX-headlen*Math.cos(angle-Math.PI/6),ToY-headlen*Math.sin(angle-Math.PI/6));
                ctx2d.moveTo(ToX, ToY);
                ctx2d.lineTo(ToX-headlen*Math.cos(angle+Math.PI/6),ToY-headlen*Math.sin(angle+Math.PI/6));
                ctx2d.stroke();
            } 

            if(pt_object.hasOwnProperty('gesture_world_coordinates'))
            {
                var WorldOriginX = pt_object.gesture_world_coordinates.origin.x.toFixed(4);
                var WorldOriginY = pt_object.gesture_world_coordinates.origin.y.toFixed(4);
                var WorldOriginZ = pt_object.gesture_world_coordinates.origin.z.toFixed(4);

                var WorldDirectionX = pt_object.gesture_world_coordinates.direction.x.toFixed(4);
                var WorldDirectionY = pt_object.gesture_world_coordinates.direction.y.toFixed(4);
                var WorldDirectionZ = pt_object.gesture_world_coordinates.direction.z.toFixed(4);

                if( Math.abs(this.prevX-WorldDirectionX)>0.05 || Math.abs(this.prevY-WorldDirectionY)>0.05 || Math.abs(this.prevZ-WorldDirectionZ)>0.05)//To Reduce print overflow
                {
                    this.prevX=WorldDirectionX;
                    this.prevY=WorldDirectionY;
                    this.prevZ=WorldDirectionZ;
                    gesture_result = true;
                }

                data_final += ";" + WorldOriginX + ", " + WorldOriginY + ", " + WorldOriginZ + ";" + WorldDirectionX + ", " + WorldDirectionY + ", " + WorldDirectionZ;
            }

            if( (tracking_result == true) || (head_pose_result == true) || (orientation_result == true) || (gesture_result == true))
                insertDataInTableRows(data_final);
        }        
 
    }
};

transporter.onClearData = (timestamp) => {
    //deleteTableRow(); //Deteting the previously printed rows

    var canvas2d=document.getElementById("color-2d-overlay");
    if (canvas2d == null) {
        console.log("Failed to get color-2d-overlay element");
    } 

    var ctx2d = canvas2d.getContext("2d");
    if (ctx2d == null)    {
        console.log("Failed to get 2d context from color-2d-overlay element");
    } 

    ctx2d.clearRect(0, 0, canvas2d.width, canvas2d.height);    // Clear the overlay canvas

    //var data = getCurrentTime(timestamp) + ", " + "Num people in frame: 0";
    //insertDataInTableRows(data);

};

transporter.onColorFrame = (width, height, data) => {
    rgb_view.updateBuffer(data, width, height, THREE.RGBFormat);
    requestAnimationFrame(rgbRenderCall);
};

function insertDataInTableRows(data) {
    var entry = data.split(";");
    var table = document.getElementById("tBody");
    var currTableRows = table.rows.length;
    
    if(currTableRows === maxTableRows){
        //console.log("Table has max rows. Deleting one row");
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
    var h = zeroPadding(date.getHours(), 2);
    var m = zeroPadding(date.getMinutes(), 2);
    var s = zeroPadding(date.getSeconds(), 2);
    var ms = zeroPadding(date.getMilliseconds(), 3);
    return(h + ":" + m + ":" + s + ":" + ms);
}

setTimeout(function() {
    transporter.open();
}, 100);
