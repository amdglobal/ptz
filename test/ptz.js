"use strict";

const ptz = require("../lib/ptz");

describe("ptz", () => {
  it("listDevices promise", () => {
    const devices = ptz.listDevices();
    expect(devices).to.exist;
    expect(devices).toBe.instanceof(Array);
  });

  it("listDevices callback", (done) => {
    const devices = ptz.listDevices();
    expect(err).toBeUndefined();
    expect(devices).not.toBeUndefined();
    expect(devices).toBe.instanceof(Array);
  });

  it("getCamera", () => {
    var camera = ptz.getCamera();
    expect(camera).not.toBeUndefined();
  });
});
