// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
/**
 * @param {string} websocketUrl - endpoint to connect e.g. 'ws://foo:8080/'
 */
function SpTransport(websocketUrl) {
    this._url = websocketUrl;
    this.onOpen = () => {};
    this.onClose = () => {};

    //this.onFisheyeFrame = (timestamp, width, height, data) => {};
    this.onColorFrame = (width, height, data) => {};
    this.onPTDataUpdate = (timestamp, pt_data) => {};
    this.onClearData = (timestamp) => {};
     /** @param {Int32Array} buf */
/*    this.onMapUpdate = (scale_mm, buf) => {}; */

    /** @param fpsupdate - update! */
/*    this.onFpsUpdate = (fpsupdate) => {}; */

    this.onEvent = (message) => {};
}

/** @param {string | object} message - serialize and send over ws */
SpTransport.prototype.sendMessage = function(message) {
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
    const MSG_RGB = 3;
    const MSG_PTINFO = 4;
    const MSG_ORINFO = 5;
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
        switch (messageType) {
            case MSG_RGB:
                this.lastColorData = message;
                //console.log("Received RGB data");
                let dv2 = new DataView(message.data, 0, 16);
                let width2 = dv2.getUint16(2, true);
                let height2 = dv2.getUint16(4, true);
                let format2 = dv2.getUint8(1);
                let ts2 = dv2.getUint32(8, true) +
                         dv2.getUint32(12, true) * TWO_TO_THE_32;
                let imageData2 = new Uint8Array(message.data, 16);
                this.ackMessage(messageType);

                //console.timeStamp('MSG_RGB');
                if (format2 === FrameFormat.Jpeg) {
                    let tss = ts2/1000 | 0;
                    //console.time('decodeJpeg'+ ts2);
                    decodeJpeg(imageData2, (width2, height2, numComponents, decodedImageData2) => {
                        //console.timeEnd('decodeJpeg'+ts2);

                        this.onColorFrame(width2, height2, decodedImageData2);
                    });
                } else {
                    // raw image
                    this.onColorFrame(width2, height2, imageData2);
                }
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
                case "person_tracking":
                    console.log("Received PT data: ", msg);
                    this.onPTDataUpdate(new Date(), msg);
                    break;
                default:
                    this.onClearData(new Date());
                    break;
            }
        }
    } catch (e) {
        console.error("error parsing message: ", e);
    }
};

SpTransport.prototype.open = function() {
    console.log("Calling SpTransport open");
    //if (this._ws) return;

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
    console.log("Calling SpTransport close");
    if (!this._ws) return;
    this._ws.close();
};
