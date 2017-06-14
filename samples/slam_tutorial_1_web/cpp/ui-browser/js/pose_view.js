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
        resetView: () => {
            mapview.cameraControls.reset();
        },
        reset: () => {
            viewModel.tracking = 4;
            transporter.sendMessage({ type: "control", command: "reset" });
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
    omap.reset();
    orender.reset();
    mapview.reset();
    poselist = [];
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
    mapview.updatePoseDisplay(pose);
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
        if(!document.getElementById("modal")) {
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

let fisheye_view = new BufferViewer();
fisheye_view.init(document.querySelector('#fisheye-canvas'));
let fisheyeRenderCall = fisheye_view.render.bind(fisheye_view);


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


setTimeout(function() {
    transporter.open();
}, 100);
