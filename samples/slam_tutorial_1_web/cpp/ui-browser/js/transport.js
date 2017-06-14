// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
/**
 * @param {string} websocketUrl - endpoint to connect e.g. 'ws://foo:8080/'
 */
function SpTransport(websocketUrl) {
    this._url = websocketUrl;
    this.loaderRemoved = false;
    this.onOpen = () => {};
    this.onClose = () => {};

    /**
     * @param {number} timestamp
     * @param {number} width
     * @param {number} height
     * @param {Uint8Array} data */
    this.onFisheyeFrame = (timestamp, width, height, data) => {};
    this.onPoseUpdate = (pose) => {};
     /** @param {Int32Array} buf */
    this.onMapUpdate = (scale_mm, buf) => {};

    /** @param fpsupdate - update! */
    this.onFpsUpdate = (fpsupdate) => {};

    this.onEvent = (message) => {};
    this.removeLoader = () => {};
}

/** @param {string | object} message - serialize and send over ws */
SpTransport.prototype.sendMessage = function(message) {
    //if (this._ws.readyState !== WebSocket.OPEN) return;
    this._ws.send(message instanceof Object ? JSON.stringify(message) : message);
};

SpTransport.prototype.ackMessage = function ack(type) {
    const MSG_ACK = 0xff;
    let msg = new Uint8Array(2);
    msg[0] = MSG_ACK;
    msg[1] = type;
    this._ws.send(msg);
};

SpTransport.prototype.handleMessageBinary = (function() {
    const MSG_MAP_UPDATE = 1;
    const MSG_FISHEYE = 2;
    const TWO_TO_THE_32 = Math.pow(2, 32);

    const FrameFormat = {
        Raw: 0,
        Jpeg: 1
    };

    const decoder = new JpegDecoder();
    function decodeJpeg(data, callback) {
        decoder.parse(data);
        var numComponents = decoder.numComponents;
        var downscale_f = 1;
        var width = decoder.width/downscale_f;
        var height = decoder.height/downscale_f;
        var img = decoder.getData(width, height, true, true);
        callback(width, height, numComponents, img);
    }

    return function(message) {
        if (message.data.byteLength < 1) return;
        // first byte is type
        let messageType = new Uint8Array(message.data, 0, 1)[0];
        //console.log("Recieved Message: type", messageType, " lenght", message.data.byteLength);
        switch (messageType) {
            case MSG_FISHEYE:
                this.lastFishData = message;
                let dv = new DataView(message.data, 0, 16);
                let width = dv.getUint16(2, true);
                let height = dv.getUint16(4, true);
                let format = dv.getUint8(1);
                let ts = dv.getUint32(8, true) +
                         dv.getUint32(12, true) * TWO_TO_THE_32;
                let imageData = new Uint8Array(message.data, 16);

                this.ackMessage(messageType);
                //console.timeStamp('MSG_FISHEYE');
                if (format === FrameFormat.Jpeg) {
                    let tss = ts/1000 | 0;
                    //console.time('decodeJpeg'+ ts);
                    decodeJpeg(imageData, (width, height, numComponents, decodedImageData) => {
                      //  console.timeEnd('decodeJpeg'+ts);

                        this.onFisheyeFrame(ts, width, height, decodedImageData);
                    });
                } else {
                    // raw image
                    this.onFisheyeFrame(ts, width, height, imageData);
                }
                if(!this.loaderRemoved)
                    this.removeLoader();
                break;
            case MSG_MAP_UPDATE:
                let scale_mm = new Uint16Array(message.data, 2, 1)[0];
                this.ackMessage(messageType);
                this.onMapUpdate(scale_mm, new Int32Array(message.data, 4));
                break;
            default:
                console.info("SpTransport: unhandled message type="+messageType);
                break;
        }
    };
})();

SpTransport.prototype.handleMessageString = function(message) {
    try {
        let msg = JSON.parse(message.data);
        if (msg instanceof Object) {
            let type = (msg.hasOwnProperty('type')) ? msg.type : undefined;
            switch (type) {
                case "tracking":
                    this.onPoseUpdate(msg);
                    break;
                case "fps":
                    this.onFpsUpdate(msg);
                    break;
                case "event":
                    this.onEvent(msg.event, msg);
                    break;
                default:
                    break;
            }
        }
    } catch (e) {
        console.error("error parsing message: ", e);
    }
};

SpTransport.prototype.open = function() {
    // if (this._ws) return;

    let ws = new WebSocket(this._url);
	ws.binaryType = "arraybuffer";
	ws.onclose = () => {
        this.onClose(this);
	};
	ws.onopen = () => {
        this.onOpen(this);
	};
	ws.onmessage = (message) => {
		if (message.data instanceof ArrayBuffer) {
            this.handleMessageBinary(message);
		} else {
            this.handleMessageString(message);
        }
	};

    /** @param {WebSocket} _ws */
    this._ws = ws;
};

SpTransport.prototype.close = function() {
    if (!this._ws) return;
    this._ws.close();
};
