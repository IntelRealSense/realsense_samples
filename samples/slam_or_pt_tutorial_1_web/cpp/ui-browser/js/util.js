// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
function Emitter(doNext, period) {
    this.period = period || 1000;
    this.doNext = doNext;
    this.listener = null;
}

Emitter.prototype.setListener = function(listener) {
    this.listener = listener;
};

Emitter.prototype.next = function() {
    var ret = this.doNext();
    if (this.listener) this.listener.onNext(ret);
    return ret;
};

Emitter.prototype.started = function() {
    return !!this._interval;
};

Emitter.prototype.start = function() {
    if (this._interval) return;
    var self = this;
    this._interval = setInterval(function() { self.next(); }, this.period);
    if (this.listener) this.listener.onStart();
};

Emitter.prototype.stop = function() {
    if (this._interval) clearInterval(this._interval);
    this._interval = null;
    if (this.listener) this.listener.onStop();
};


class EventTimer {
    constructor() {
        this.start = -1;
    }

    get time() {
        let now = performance.now();
        if (this.start < 0) {
            this.start = now;
        }
        return now - this.start;
    }
}

class EventRecorder {
    constructor() {
        this.timer = new EventTimer();
        this.events = new Map();
    }

    save(key, evt) {
        let ts = this.timer.time;
        let e = this.events.get(key);
        if (e === undefined) {
            e = {ts: [], data: []};
            this.events.set(key, e);
        }
        e.ts.push(ts);
        e.data.push(evt);
    }

    getTimestampedEvents(key, setTypeField=false) {
        let e = this.events.get(key);
        return e.data.map(
            (x,i) => {
                let obj = {event: x, ts: e.ts[i]};
                if (setTypeField) obj.type = key;
                return obj;
        });
    }

    getAllEvents() {
        let all =[];
        this.events.forEach((val, type) =>
            all = all.concat(this.getTimestampedEvents(type, true)));

        return all.sort((a,b) => a.ts - b.ts);
    }
}




function testEventRecroder() {
    let counter = 0;
    function doSave(key, value) {
        timeout = Math.random() * 5 + 10*(counter++);
        setTimeout(() => rec.save(key, value), timeout);
    }

    doSave('foo', 1);
    doSave('foo', 2);
    doSave('bar', new Date());
    doSave('foo', 2);
    doSave('bar', new Date());
    doSave('bar', new Date());
    doSave('bar', new Date());
    let all = rec.getAllEvents();
}

function savePoses() {
    if (websocket_poses.length < 1) {
        alert("no poses");
        return;
    }
    var now = new Date();
    var dstr = now.getFullYear() +"-" + now.getDate() + "-" + now.getMonth() + "_" + (now.getHours()*100 + now.getSeconds());
    var blob = new Blob([JSON.stringify(websocket_poses, null, 2)], {type: "text/plain;charset=utf-8"});
    saveAs(blob, "poses"+dstr+".json");
}
