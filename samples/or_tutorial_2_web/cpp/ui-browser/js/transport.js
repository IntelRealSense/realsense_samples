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
    this.onColorFrame = (timestamp, width, height, data, orInfo) => {};
    this.onPoseUpdate = (pose) => {};
    this.onORDataUpdate = (or_data) => {};
    this.onClearData = (timestamp) => {};
     /** @param {Int32Array} buf */
    this.onMapUpdate = (scale_mm, buf) => {};

    /** @param fpsupdate - update! */
    this.onFpsUpdate = (fpsupdate) => {};

    this.onEvent = (message) => {};
    this.onORLabel = (or_label) => {};
    this.removeLoader = () => {};
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
    const MSG_RGB = 3;
    const MSG_ORINFO = 2;
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
        if (message.data.byteLength < 1)
            return;

        let messageType = new Uint8Array(message.data, 0, 1)[0];        // first byte is type
        switch (messageType) {

            case MSG_RGB:
                this.lastColorData = message;
                let dv2 = new DataView(message.data, 0, 16);
                let width2 = dv2.getUint16(2, true);
                let height2 = dv2.getUint16(4, true);
                let format2 = dv2.getUint8(1);
                let ts2 = dv2.getUint32(8, true) +
                         dv2.getUint32(12, true) * TWO_TO_THE_32;
                let imageData2 = new Uint8Array(message.data, 16);
                this.ackMessage(messageType);
                var difference = new Date().getTime() - this.lasttimeStamp;


                if (format2 === FrameFormat.Jpeg) {
                    let tss = ts2/1000 | 0;
                    decodeJpeg(imageData2, (width2, height2, numComponents, decodedImageData2) => {

                        this.onColorFrame(ts2, width2, height2, decodedImageData2, this.lastORData);
                    });
                } else {
                   // Raw image
                    this.onColorFrame(ts2, width2, height2, imageData2, this.lastORData);
                }
                if(!this.loaderRemoved){
                    this.removeLoader();
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
                case "object_localization":
                    this.lastORData = msg;
                    this.lasttimeStamp = new Date().getTime();
                    this.onORInfo(new Date(), msg);
                    break;
                case "object_recognition_label_list":
                    this.onORLabel(msg);
                    break;
                case "object_localization_none":
                    this.lastORData = undefined;
                default:
                    break;
            }
        }
    } catch (e) {
        console.error("error parsing message: ", e);
    }
};

SpTransport.prototype.open = function() {
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
