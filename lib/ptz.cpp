#include <nan.h>
#include <iostream>
#include <ostream>
#include <string>
#include "libuvc/libuvc.h"

namespace ptz {

using Nan::FunctionCallbackInfo;
using Nan::GetFunction;
using Nan::New;
using Nan::Set;
using Nan::SetMethod;
using v8::Array;
using v8::Boolean;
using v8::Function;
using v8::FunctionTemplate;
using v8::Int32;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Null;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Undefined;
using v8::Value;

// open close device operations
struct UVCDevice {
    uvc_context_t*       ctx;
    uvc_device_t*        device;
    uvc_device_handle_t* devicehandle;
    uvc_error_t          result;
    const char*          error;
    int                  vendorId;
    int                  productId;
};

void trySetting(const Local<Object>& target, const std::string& key, const char* value) {
    Local<Value> v8Key = Nan::New<String>(key).ToLocalChecked();

    if (value != NULL) {
        Nan::Set(target, v8Key, Nan::New<String>(value).ToLocalChecked());
    } else {
        Nan::Set(target, v8Key, Nan::Null());
    }
}

void openDevice(UVCDevice* uvcDevice) {
    // initalize uvc
    uvcDevice->result = uvc_init(&uvcDevice->ctx, NULL);
    if (uvcDevice->result != 0) {
        uvc_exit(uvcDevice->ctx);
        std::string error(uvc_strerror(uvcDevice->result));
        uvcDevice->error = error.c_str();
    }

    // get device
    uvcDevice->result = uvc_find_device(
        uvcDevice->ctx, &uvcDevice->device, uvcDevice->vendorId, uvcDevice->productId, NULL);
    if (uvcDevice->result != 0) {
        uvc_unref_device(uvcDevice->device);
        uvc_exit(uvcDevice->ctx);
        std::string error(uvc_strerror(uvcDevice->result));
        uvcDevice->error = error.c_str();
    }

    // open device
    uvcDevice->result = uvc_open(uvcDevice->device, &uvcDevice->devicehandle);
    if (uvcDevice->result != 0) {
        uvc_close(uvcDevice->devicehandle);
        uvc_unref_device(uvcDevice->device);
        uvc_exit(uvcDevice->ctx);
        std::string error(uvc_strerror(uvcDevice->result));
        uvcDevice->error = error.c_str();
    }
}

void closeDevice(UVCDevice* uvcDevice) {
    uvc_close(uvcDevice->devicehandle);
    uvc_unref_device(uvcDevice->device);
    uvc_exit(uvcDevice->ctx);
}

// device operation support check
struct DeviceCapability {
    int absolute_zoom;
    int relative_zoom;
    int absolute_pan_tilt;
    int relative_pan_tilt;
    int absolute_roll;
    int relative_roll;
};

struct DeviceCapability getDeviceCapability(struct UVCDevice* uvcDevice) {
    struct DeviceCapability deviceCapability;
    enum uvc_req_code       requestCode;
    uvc_error_t             res;
    int32_t                 v1;
    int32_t                 v2;
    uint16_t                v3;
    int8_t                  v4;
    uint8_t                 v5;
    uint8_t                 v6;
    uint8_t                 v7;
    int16_t                 v8;
    int8_t                  v10;

    requestCode                    = UVC_GET_DEF;
    res                            = uvc_get_zoom_abs(uvcDevice->devicehandle, &v3, requestCode);
    deviceCapability.absolute_zoom = res == 0 ? 1 : 0;

    requestCode = UVC_GET_DEF;
    res         = uvc_get_zoom_rel(uvcDevice->devicehandle, &v4, &v5, &v6, requestCode);
    deviceCapability.relative_zoom = res == 0 ? 1 : 0;

    requestCode = UVC_GET_DEF;
    res         = uvc_get_pantilt_abs(uvcDevice->devicehandle, &v1, &v2, requestCode);
    deviceCapability.absolute_pan_tilt = res == 0 ? 1 : 0;

    requestCode = UVC_GET_DEF;
    res         = uvc_get_pantilt_rel(uvcDevice->devicehandle, &v4, &v5, &v10, &v7, requestCode);
    deviceCapability.relative_pan_tilt = res == 0 ? 1 : 0;

    requestCode                    = UVC_GET_DEF;
    res                            = uvc_get_roll_abs(uvcDevice->devicehandle, &v8, requestCode);
    deviceCapability.absolute_roll = res == 0 ? 1 : 0;

    requestCode = UVC_GET_DEF;
    res         = uvc_get_roll_rel(uvcDevice->devicehandle, &v4, &v5, requestCode);
    deviceCapability.relative_roll = res == 0 ? 1 : 0;

    return deviceCapability;
}

// absolute zoom operations
struct AbsoluteZoomInfo {
    uint16_t    min;
    uint16_t    max;
    uint16_t    resolution;
    uint16_t    current;
    uint16_t    def;
    uvc_error_t result;
    const char* error;
};

void getAbsoluteZoomInfo(struct UVCDevice* uvcDevice, struct AbsoluteZoomInfo* absoluteZoomInfo) {
    enum uvc_req_code requestCode;

    requestCode              = UVC_GET_MIN;
    absoluteZoomInfo->result = uvc_get_zoom_abs(
        uvcDevice->devicehandle, &absoluteZoomInfo->min, requestCode);
    if (absoluteZoomInfo->result != 0) {
        std::string error(uvc_strerror(absoluteZoomInfo->result));
        absoluteZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_MAX;
    absoluteZoomInfo->result = uvc_get_zoom_abs(
        uvcDevice->devicehandle, &absoluteZoomInfo->max, requestCode);
    if (absoluteZoomInfo->result != 0) {
        std::string error(uvc_strerror(absoluteZoomInfo->result));
        absoluteZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_RES;
    absoluteZoomInfo->result = uvc_get_zoom_abs(
        uvcDevice->devicehandle, &absoluteZoomInfo->resolution, requestCode);
    if (absoluteZoomInfo->result != 0) {
        std::string error(uvc_strerror(absoluteZoomInfo->result));
        absoluteZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_DEF;
    absoluteZoomInfo->result = uvc_get_zoom_abs(
        uvcDevice->devicehandle, &absoluteZoomInfo->def, requestCode);
    if (absoluteZoomInfo->result != 0) {
        std::string error(uvc_strerror(absoluteZoomInfo->result));
        absoluteZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_CUR;
    absoluteZoomInfo->result = uvc_get_zoom_abs(
        uvcDevice->devicehandle, &absoluteZoomInfo->current, requestCode);
    if (absoluteZoomInfo->result != 0) {
        std::string error(uvc_strerror(absoluteZoomInfo->result));
        absoluteZoomInfo->error = error.c_str();
        return;
    }
}

struct AbsoluteZoom {
    uint16_t    zoom;
    uvc_error_t result;
    const char* error;
};

void setAbsoluteZoom(struct UVCDevice* uvcDevice, struct AbsoluteZoom* absoluteZoom) {
    absoluteZoom->result = uvc_set_zoom_abs(uvcDevice->devicehandle, absoluteZoom->zoom);
    if (absoluteZoom->result != 0) {
        std::string error(uvc_strerror(absoluteZoom->result));
        absoluteZoom->error = error.c_str();
        return;
    }
}

// relative zoom operations
struct RelativeZoomInfo {
    int8_t      direction;
    uint8_t     digital_zoom;
    uint8_t     min_speed;
    uint8_t     max_speed;
    uint8_t     resolution_speed;
    uint8_t     current_speed;
    uint8_t     default_speed;
    uvc_error_t result;
    const char* error;
};

void getRelativeZoomInfo(struct UVCDevice* uvcDevice, struct RelativeZoomInfo* relativeZoomInfo) {
    enum uvc_req_code requestCode;

    requestCode              = UVC_GET_MIN;
    relativeZoomInfo->result = uvc_get_zoom_rel(uvcDevice->devicehandle,
                                                &relativeZoomInfo->direction,
                                                &relativeZoomInfo->digital_zoom,
                                                &relativeZoomInfo->min_speed,
                                                requestCode);
    if (relativeZoomInfo->result != 0) {
        std::string error(uvc_strerror(relativeZoomInfo->result));
        relativeZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_MAX;
    relativeZoomInfo->result = uvc_get_zoom_rel(uvcDevice->devicehandle,
                                                &relativeZoomInfo->direction,
                                                &relativeZoomInfo->digital_zoom,
                                                &relativeZoomInfo->max_speed,
                                                requestCode);
    if (relativeZoomInfo->result != 0) {
        std::string error(uvc_strerror(relativeZoomInfo->result));
        relativeZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_RES;
    relativeZoomInfo->result = uvc_get_zoom_rel(uvcDevice->devicehandle,
                                                &relativeZoomInfo->direction,
                                                &relativeZoomInfo->digital_zoom,
                                                &relativeZoomInfo->resolution_speed,
                                                requestCode);
    if (relativeZoomInfo->result != 0) {
        std::string error(uvc_strerror(relativeZoomInfo->result));
        relativeZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_DEF;
    relativeZoomInfo->result = uvc_get_zoom_rel(uvcDevice->devicehandle,
                                                &relativeZoomInfo->direction,
                                                &relativeZoomInfo->digital_zoom,
                                                &relativeZoomInfo->default_speed,
                                                requestCode);
    if (relativeZoomInfo->result != 0) {
        std::string error(uvc_strerror(relativeZoomInfo->result));
        relativeZoomInfo->error = error.c_str();
        return;
    }

    requestCode              = UVC_GET_CUR;
    relativeZoomInfo->result = uvc_get_zoom_rel(uvcDevice->devicehandle,
                                                &relativeZoomInfo->direction,
                                                &relativeZoomInfo->digital_zoom,
                                                &relativeZoomInfo->current_speed,
                                                requestCode);
    if (relativeZoomInfo->result != 0) {
        std::string error(uvc_strerror(relativeZoomInfo->result));
        relativeZoomInfo->error = error.c_str();
        return;
    }
}

struct RelativeZoom {
    int8_t      direction;
    int8_t      speed;
    uvc_error_t result;
    const char* error;
};

void setRelativeZoom(struct UVCDevice* uvcDevice, struct RelativeZoom* relativeZoom) {
    relativeZoom->result = uvc_set_zoom_rel(
        uvcDevice->devicehandle, relativeZoom->direction, 1, relativeZoom->speed);
    if (relativeZoom->result != 0) {
        std::string error(uvc_strerror(relativeZoom->result));
        relativeZoom->error = error.c_str();
        return;
    }
}

// absolute pan tilt operations
struct AbsolutePanTiltInfo {
    int32_t     min_pan;
    int32_t     min_tilt;
    int32_t     max_pan;
    int32_t     max_tilt;
    int32_t     resolution_pan;
    int32_t     resolution_tilt;
    int32_t     current_pan;
    int32_t     current_tilt;
    int32_t     default_pan;
    int32_t     default_tilt;
    uvc_error_t result;
    const char* error;
};

void getAbsolutePanTiltInfo(struct UVCDevice*           uvcDevice,
                            struct AbsolutePanTiltInfo* absolutePanTiltInfo) {
    enum uvc_req_code requestCode;

    requestCode                 = UVC_GET_MIN;
    absolutePanTiltInfo->result = uvc_get_pantilt_abs(uvcDevice->devicehandle,
                                                      &absolutePanTiltInfo->min_pan,
                                                      &absolutePanTiltInfo->min_tilt,
                                                      requestCode);
    if (absolutePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(absolutePanTiltInfo->result));
        absolutePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_MAX;
    absolutePanTiltInfo->result = uvc_get_pantilt_abs(uvcDevice->devicehandle,
                                                      &absolutePanTiltInfo->max_pan,
                                                      &absolutePanTiltInfo->max_pan,
                                                      requestCode);
    if (absolutePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(absolutePanTiltInfo->result));
        absolutePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_RES;
    absolutePanTiltInfo->result = uvc_get_pantilt_abs(uvcDevice->devicehandle,
                                                      &absolutePanTiltInfo->resolution_pan,
                                                      &absolutePanTiltInfo->resolution_tilt,
                                                      requestCode);
    if (absolutePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(absolutePanTiltInfo->result));
        absolutePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_DEF;
    absolutePanTiltInfo->result = uvc_get_pantilt_abs(uvcDevice->devicehandle,
                                                      &absolutePanTiltInfo->default_pan,
                                                      &absolutePanTiltInfo->default_tilt,
                                                      requestCode);
    if (absolutePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(absolutePanTiltInfo->result));
        absolutePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_CUR;
    absolutePanTiltInfo->result = uvc_get_pantilt_abs(uvcDevice->devicehandle,
                                                      &absolutePanTiltInfo->current_pan,
                                                      &absolutePanTiltInfo->current_tilt,
                                                      requestCode);
    if (absolutePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(absolutePanTiltInfo->result));
        absolutePanTiltInfo->error = error.c_str();
        return;
    }
}

struct AbsolutePanTilt {
    int32_t     pan;
    int32_t     tilt;
    uvc_error_t result;
    const char* error;
};

void setAbsolutePanTilt(struct UVCDevice* uvcDevice, struct AbsolutePanTilt* absolutePanTilt) {
    absolutePanTilt->result = uvc_set_pantilt_abs(
        uvcDevice->devicehandle, absolutePanTilt->pan, absolutePanTilt->tilt);
    if (absolutePanTilt->result != 0) {
        std::string error(uvc_strerror(absolutePanTilt->result));
        absolutePanTilt->error = error.c_str();
        return;
    }
}

// relative pan tilt operations
struct RelativePanTiltInfo {
    int8_t      pan_direction;
    int8_t      tilt_direction;
    uint8_t     min_pan_speed;
    uint8_t     min_tilt_speed;
    uint8_t     max_pan_speed;
    uint8_t     max_tilt_speed;
    uint8_t     resolution_pan_speed;
    uint8_t     resolution_tilt_speed;
    uint8_t     default_pan_speed;
    uint8_t     default_tilt_speed;
    uint8_t     current_pan_speed;
    uint8_t     current_tilt_speed;
    uvc_error_t result;
    const char* error;
};

void getRelativePanTiltInfo(struct UVCDevice*           uvcDevice,
                            struct RelativePanTiltInfo* relativePanTiltInfo) {
    enum uvc_req_code requestCode;

    requestCode                 = UVC_GET_MIN;
    relativePanTiltInfo->result = uvc_get_pantilt_rel(uvcDevice->devicehandle,
                                                      &relativePanTiltInfo->pan_direction,
                                                      &relativePanTiltInfo->min_pan_speed,
                                                      &relativePanTiltInfo->tilt_direction,
                                                      &relativePanTiltInfo->min_tilt_speed,
                                                      requestCode);
    if (relativePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(relativePanTiltInfo->result));
        relativePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_MAX;
    relativePanTiltInfo->result = uvc_get_pantilt_rel(uvcDevice->devicehandle,
                                                      &relativePanTiltInfo->pan_direction,
                                                      &relativePanTiltInfo->max_pan_speed,
                                                      &relativePanTiltInfo->tilt_direction,
                                                      &relativePanTiltInfo->max_tilt_speed,
                                                      requestCode);
    if (relativePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(relativePanTiltInfo->result));
        relativePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_RES;
    relativePanTiltInfo->result = uvc_get_pantilt_rel(uvcDevice->devicehandle,
                                                      &relativePanTiltInfo->pan_direction,
                                                      &relativePanTiltInfo->resolution_pan_speed,
                                                      &relativePanTiltInfo->tilt_direction,
                                                      &relativePanTiltInfo->resolution_tilt_speed,
                                                      requestCode);
    if (relativePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(relativePanTiltInfo->result));
        relativePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_DEF;
    relativePanTiltInfo->result = uvc_get_pantilt_rel(uvcDevice->devicehandle,
                                                      &relativePanTiltInfo->pan_direction,
                                                      &relativePanTiltInfo->default_pan_speed,
                                                      &relativePanTiltInfo->tilt_direction,
                                                      &relativePanTiltInfo->default_tilt_speed,
                                                      requestCode);
    if (relativePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(relativePanTiltInfo->result));
        relativePanTiltInfo->error = error.c_str();
        return;
    }

    requestCode                 = UVC_GET_CUR;
    relativePanTiltInfo->result = uvc_get_pantilt_rel(uvcDevice->devicehandle,
                                                      &relativePanTiltInfo->pan_direction,
                                                      &relativePanTiltInfo->current_pan_speed,
                                                      &relativePanTiltInfo->tilt_direction,
                                                      &relativePanTiltInfo->current_tilt_speed,
                                                      requestCode);
    if (relativePanTiltInfo->result != 0) {
        std::string error(uvc_strerror(relativePanTiltInfo->result));
        relativePanTiltInfo->error = error.c_str();
        return;
    }
}

struct RelativePanTilt {
    int8_t      pan_direction;
    uint8_t     pan_speed;
    int8_t      tilt_direction;
    uint8_t     tilt_speed;
    uvc_error_t result;
    const char* error;
};

void setRelativePanTilt(struct UVCDevice* uvcDevice, struct RelativePanTilt* relativePanTilt) {
    relativePanTilt->result = uvc_set_pantilt_rel(uvcDevice->devicehandle,
                                                  relativePanTilt->pan_direction,
                                                  relativePanTilt->pan_speed,
                                                  relativePanTilt->tilt_direction,
                                                  relativePanTilt->tilt_speed);
    if (relativePanTilt->result != 0) {
        std::string error(uvc_strerror(relativePanTilt->result));
        relativePanTilt->error = error.c_str();
        return;
    }
}

// node binding functions
NAN_METHOD(listDevices) {
    // get device list
    uvc_context_t*           ctx;
    uvc_device_t**           devices;
    size_t                   device_count;
    uvc_device_descriptor_t* descriptor;
    uvc_error_t              res;

    // initalize uvc
    res = uvc_init(&ctx, NULL);
    if (res != 0) {
        uvc_exit(ctx);
        std::string error(uvc_strerror(res));
        Nan::ThrowError(error.c_str());
        return;
    }

    // get devices list
    res = uvc_get_device_list(ctx, &devices);

    if (res != 0) {
        uvc_exit(ctx);
        std::string error(uvc_strerror(res));
        Nan::ThrowError(error.c_str());
        return;
    }

    device_count = 0;
    while (devices[device_count++] != NULL)
        continue;

    // create output result
    Local<Array> jsDevices = Nan::New<Array>();
    for (int i = 0; devices[i] != NULL; i++) {
        std::cout << "i: " << i << std::endl;
        uvc_device_t* device = devices[i];
        std::cout << "device: " << device << std::endl << std::flush;
        res = uvc_get_device_descriptor(device, &descriptor);
        if (res != UVC_SUCCESS) {
            uvc_free_device_list(devices, 1);
            uvc_exit(ctx);
            std::string error(uvc_strerror(res));
            Nan::ThrowError(error.c_str());
            return;
        }
        Local<Object> jsDevice = Nan::New<Object>();

        std::cout << "vendorId: " << descriptor->idVendor << std::endl
                  << "productId: " << descriptor->idProduct << std::endl
                  << "serialNumber: " << descriptor->serialNumber << std::endl
                  << "manufacturer: " << descriptor->manufacturer << std::endl
                  << "product: " << descriptor->product << std::endl
                  << std::flush;

        Nan::Set(jsDevice,
                 Nan::New<String>("vendorId").ToLocalChecked(),
                 Nan::New<Uint32>((uint32_t)descriptor->idVendor));
        Nan::Set(jsDevice,
                 Nan::New<String>("productId").ToLocalChecked(),
                 Nan::New<Uint32>((uint32_t)descriptor->idProduct));

        trySetting(jsDevice, "serialNumber", descriptor->serialNumber);
        trySetting(jsDevice, "manufacturer", descriptor->manufacturer);
        trySetting(jsDevice, "product", descriptor->product);

        Nan::Set(jsDevices, i, jsDevice);

        uvc_free_device_descriptor(descriptor);
    }
    uvc_free_device_list(devices, 1);
    uvc_exit(ctx);

    info.GetReturnValue().Set(jsDevices);
}
NAN_METHOD(getCapabilities) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }
    // printf("==original2===%p====", uvcDevice.devicehandle);

    // get capabilities
    struct DeviceCapability deviceCapability;
    deviceCapability = getDeviceCapability(&uvcDevice);

    // create output result
    Local<Object> jsDevice = Nan::New<Object>();
    Nan::Set(jsDevice,
             Nan::New<String>("absoluteZoom").ToLocalChecked(),
             Nan::New<Boolean>(deviceCapability.absolute_zoom));
    Nan::Set(jsDevice,
             Nan::New<String>("relativeZoom").ToLocalChecked(),
             Nan::New<Boolean>(deviceCapability.relative_zoom));
    Nan::Set(jsDevice,
             Nan::New<String>("absolutePanTilt").ToLocalChecked(),
             Nan::New<Boolean>(deviceCapability.absolute_pan_tilt));
    Nan::Set(jsDevice,
             Nan::New<String>("relativePanTilt").ToLocalChecked(),
             Nan::New<Boolean>(deviceCapability.relative_pan_tilt));
    Nan::Set(jsDevice,
             Nan::New<String>("absoluteRoll").ToLocalChecked(),
             Nan::New<Boolean>(deviceCapability.absolute_roll));
    Nan::Set(jsDevice,
             Nan::New<String>("relativeRoll").ToLocalChecked(),
             Nan::New<Boolean>(deviceCapability.relative_roll));

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    Local<Value> argv[2] = {Nan::Undefined(), jsDevice};
    info.GetReturnValue().Set(jsDevice);
}
NAN_METHOD(getAbsoluteZoom) {
    Local<Object> input = Local<Object>::Cast(info[0]);
    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Local<Value> argv[2] = {Nan::New<String>(uvcDevice.error).ToLocalChecked(),
                                Nan::Undefined()};
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct AbsoluteZoomInfo absoluteZoomInfo;
    getAbsoluteZoomInfo(&uvcDevice, &absoluteZoomInfo);
    if (absoluteZoomInfo.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(absoluteZoomInfo.error);
        return;
    }

    // create output result
    Local<Object> jsDevice = Nan::New<Object>();
    Nan::Set(jsDevice,
             Nan::New<String>("min").ToLocalChecked(),
             Nan::New<Integer>(absoluteZoomInfo.min));
    Nan::Set(jsDevice,
             Nan::New<String>("max").ToLocalChecked(),
             Nan::New<Integer>(absoluteZoomInfo.max));
    Nan::Set(jsDevice,
             Nan::New<String>("resolution").ToLocalChecked(),
             Nan::New<Integer>(absoluteZoomInfo.resolution));
    Nan::Set(jsDevice,
             Nan::New<String>("current").ToLocalChecked(),
             Nan::New<Integer>(absoluteZoomInfo.current));
    Nan::Set(jsDevice,
             Nan::New<String>("default").ToLocalChecked(),
             Nan::New<Integer>(absoluteZoomInfo.def));

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    Local<Value> argv[2] = {Nan::Undefined(), jsDevice};
    info.GetReturnValue().Set(jsDevice);
}
NAN_METHOD(absoluteZoom) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();
    Local<Value> zoom = Nan::Get(input, Nan::New<String>("zoom").ToLocalChecked()).ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct AbsoluteZoom absoluteZoom;
    absoluteZoom.zoom = Nan::To<int32_t>(zoom).FromMaybe(0);
    setAbsoluteZoom(&uvcDevice, &absoluteZoom);
    if (absoluteZoom.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(absoluteZoom.error);
        return;
    }

    // cleanup
    closeDevice(&uvcDevice);

    info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(getRelativeZoom) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct RelativeZoomInfo relativeZoomInfo;
    getRelativeZoomInfo(&uvcDevice, &relativeZoomInfo);
    if (relativeZoomInfo.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(relativeZoomInfo.error);
        return;
    }

    // create output result
    Local<Object> result = Nan::New<Object>();
    Nan::Set(result,
             Nan::New<String>("direction").ToLocalChecked(),
             Nan::New<Integer>(relativeZoomInfo.direction));
    Nan::Set(result,
             Nan::New<String>("digitalZoom").ToLocalChecked(),
             Nan::New<Boolean>(relativeZoomInfo.digital_zoom));
    Nan::Set(result,
             Nan::New<String>("minSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativeZoomInfo.min_speed));
    Nan::Set(result,
             Nan::New<String>("maxSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativeZoomInfo.max_speed));
    Nan::Set(result,
             Nan::New<String>("resolutionSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativeZoomInfo.resolution_speed));
    Nan::Set(result,
             Nan::New<String>("currentSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativeZoomInfo.current_speed));
    Nan::Set(result,
             Nan::New<String>("defaultSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativeZoomInfo.default_speed));

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    info.GetReturnValue().Set(result);
}
NAN_METHOD(relativeZoom) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();
    Local<Value> direction = Nan::Get(input, Nan::New<String>("direction").ToLocalChecked())
                                 .ToLocalChecked();
    Local<Value> speed = Nan::Get(input, Nan::New<String>("speed").ToLocalChecked())
                             .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct RelativeZoom relativeZoom;
    relativeZoom.direction = Nan::To<int32_t>(direction).FromMaybe(0);
    relativeZoom.speed     = Nan::To<int32_t>(speed).FromMaybe(0);
    setRelativeZoom(&uvcDevice, &relativeZoom);
    if (relativeZoom.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(relativeZoom.error);
        return;
    }

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(getAbsolutePanTilt) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get pan tilt info
    struct AbsolutePanTiltInfo absolutePanTiltInfo;
    getAbsolutePanTiltInfo(&uvcDevice, &absolutePanTiltInfo);
    if (absolutePanTiltInfo.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(absolutePanTiltInfo.error);
        return;
    }

    // create output result
    Local<Object> result = Nan::New<Object>();
    Nan::Set(result,
             Nan::New<String>("minPan").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.min_pan));
    Nan::Set(result,
             Nan::New<String>("minTilt").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.min_tilt));
    Nan::Set(result,
             Nan::New<String>("maxPan").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.max_pan));
    Nan::Set(result,
             Nan::New<String>("maxTilt").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.max_tilt));
    Nan::Set(result,
             Nan::New<String>("resolutionPan").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.resolution_pan));
    Nan::Set(result,
             Nan::New<String>("resolutionTilt").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.resolution_tilt));
    Nan::Set(result,
             Nan::New<String>("currentPan").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.current_pan));
    Nan::Set(result,
             Nan::New<String>("currentTilt").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.current_tilt));
    Nan::Set(result,
             Nan::New<String>("defaultPan").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.default_pan));
    Nan::Set(result,
             Nan::New<String>("defaultTilt").ToLocalChecked(),
             Nan::New<Integer>(absolutePanTiltInfo.default_tilt));

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    info.GetReturnValue().Set(result);
}
NAN_METHOD(absolutePanTilt) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();
    Local<Value> pan  = Nan::Get(input, Nan::New<String>("pan").ToLocalChecked()).ToLocalChecked();
    Local<Value> tilt = Nan::Get(input, Nan::New<String>("tilt").ToLocalChecked()).ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct AbsolutePanTilt absolutePanTilt;
    absolutePanTilt.pan  = Nan::To<int32_t>(pan).FromMaybe(0);
    absolutePanTilt.tilt = Nan::To<int32_t>(tilt).FromMaybe(0);
    setAbsolutePanTilt(&uvcDevice, &absolutePanTilt);
    if (absolutePanTilt.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(absolutePanTilt.error);
        return;
    }

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(getRelativePanTilt) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct RelativePanTiltInfo relativePanTiltInfo;
    getRelativePanTiltInfo(&uvcDevice, &relativePanTiltInfo);
    if (relativePanTiltInfo.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(relativePanTiltInfo.error);
        return;
    }

    // create output result
    Local<Object> result = Nan::New<Object>();
    Nan::Set(result,
             Nan::New<String>("panDirection").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.pan_direction));
    Nan::Set(result,
             Nan::New<String>("tiltDirection").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.tilt_direction));
    Nan::Set(result,
             Nan::New<String>("minPanSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.min_pan_speed));
    Nan::Set(result,
             Nan::New<String>("minTiltSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.min_tilt_speed));
    Nan::Set(result,
             Nan::New<String>("maxPanSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.max_pan_speed));
    Nan::Set(result,
             Nan::New<String>("maxTiltSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.max_tilt_speed));
    Nan::Set(result,
             Nan::New<String>("resolutionPanSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.resolution_pan_speed));
    Nan::Set(result,
             Nan::New<String>("resolutionTiltSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.resolution_tilt_speed));
    Nan::Set(result,
             Nan::New<String>("defaultPanSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.default_pan_speed));
    Nan::Set(result,
             Nan::New<String>("defaultTiltSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.default_tilt_speed));
    Nan::Set(result,
             Nan::New<String>("currentPanSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.current_pan_speed));
    Nan::Set(result,
             Nan::New<String>("currentTiltSpeed").ToLocalChecked(),
             Nan::New<Integer>(relativePanTiltInfo.current_tilt_speed));

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    info.GetReturnValue().Set(result);
}
NAN_METHOD(relativePanTilt) {
    Local<Object> input = Local<Object>::Cast(info[0]);

    // get options
    Local<Value> vendorId = Nan::Get(input, Nan::New<String>("vendorId").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> productId = Nan::Get(input, Nan::New<String>("productId").ToLocalChecked())
                                 .ToLocalChecked();
    Local<Value> panDirection = Nan::Get(input, Nan::New<String>("panDirection").ToLocalChecked())
                                    .ToLocalChecked();
    Local<Value> panSpeed = Nan::Get(input, Nan::New<String>("panSpeed").ToLocalChecked())
                                .ToLocalChecked();
    Local<Value> tiltDirection = Nan::Get(input, Nan::New<String>("tiltDirection").ToLocalChecked())
                                     .ToLocalChecked();
    Local<Value> tiltSpeed = Nan::Get(input, Nan::New<String>("tiltSpeed").ToLocalChecked())
                                 .ToLocalChecked();

    struct UVCDevice uvcDevice;
    uvcDevice.vendorId  = Nan::To<int32_t>(vendorId).FromMaybe(0);
    uvcDevice.productId = Nan::To<int32_t>(productId).FromMaybe(0);

    // open device
    openDevice(&uvcDevice);
    if (uvcDevice.result != 0) {
        Nan::ThrowError(uvcDevice.error);
        return;
    }

    // get zoom info
    struct RelativePanTilt relativePanTilt;
    relativePanTilt.pan_direction  = Nan::To<int32_t>(panDirection).FromMaybe(0);
    relativePanTilt.pan_speed      = Nan::To<int32_t>(panSpeed).FromMaybe(0);
    relativePanTilt.tilt_direction = Nan::To<int32_t>(tiltDirection).FromMaybe(0);
    relativePanTilt.tilt_speed     = Nan::To<int32_t>(tiltSpeed).FromMaybe(0);
    setRelativePanTilt(&uvcDevice, &relativePanTilt);
    if (relativePanTilt.result != 0) {
        closeDevice(&uvcDevice);
        Nan::ThrowError(relativePanTilt.error);
        return;
    }

    // cleanup
    closeDevice(&uvcDevice);

    // trigger callback
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_MODULE_INIT(InitAll) {
    NAN_EXPORT(target, listDevices);
    NAN_EXPORT(target, getCapabilities);
    NAN_EXPORT(target, getAbsoluteZoom);
    NAN_EXPORT(target, absoluteZoom);
    NAN_EXPORT(target, getRelativeZoom);
    NAN_EXPORT(target, relativeZoom);
    NAN_EXPORT(target, getAbsolutePanTilt);
    NAN_EXPORT(target, absolutePanTilt);
    NAN_EXPORT(target, getRelativePanTilt);
    NAN_EXPORT(target, relativePanTilt);

    // SetMethod(target, "listDevices", listDevices);
    // SetMethod(target, "getCapabilities", getCapabilities);
    // SetMethod(target, "getAbsoluteZoom", getAbsoluteZoom);
    // SetMethod(target, "absoluteZoom", absoluteZoom);
    // SetMethod(target, "getRelativeZoom", getRelativeZoom);
    // SetMethod(target, "relativeZoom", relativeZoom);
    // SetMethod(target, "getAbsolutePanTilt", getAbsolutePanTilt);
    // SetMethod(target, "absolutePanTilt", absolutePanTilt);
    // SetMethod(target, "getRelativePanTilt", getRelativePanTilt);
    // SetMethod(target, "relativePanTilt", relativePanTilt);
}

NODE_MODULE(ptz, InitAll);
}  // namespace ptz
