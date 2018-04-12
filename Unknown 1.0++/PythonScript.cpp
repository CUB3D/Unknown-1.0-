#include "stdafx.h"
#include "PythonScript.h"
#include <Python.h>
#include "Log.h"
#include "Unknown.h"
#include <cstdio>
#include "Loader.h"
#include "Image.h"
#include "Event/EventManager.h"

Unknown::Python::Interpreter* ::Unknown::Python::instance = NULL;

void Unknown::Python::Interpreter::checkError(PyObject* obj)
{
    if(obj == NULL)
    {
        PyErr_Print();
        exit(0);
    }
}

void Unknown::Python::Interpreter::addSearchPath(std::string name)
{
    PyObject* sysModulePath = PyObject_GetAttrString(this->moduleSys, "path");
    checkError(sysModulePath);
    PyList_Append(sysModulePath, PyUnicode_FromString(name.c_str()));
}

void Unknown::Python::Interpreter::init()
{
    Py_Initialize();

    this->moduleSys = PyImport_ImportModule("sys");
    checkError(this->moduleSys);
    this->addSearchPath(".");
}

Unknown::Python::Interpreter* Unknown::Python::getInterpreter()
{
    if(instance == NULL)
    {
        instance = new Interpreter();
        instance->init();
    }
    return instance;
}

PyMethodDef* generatePyMethodDef(std::string functionName, PyCFunction implementation, std::string doc)
{
    PyMethodDef* definition = (PyMethodDef*) malloc(sizeof(*definition));
    definition->ml_name = functionName.c_str();
    definition->ml_meth = implementation;
    definition->ml_flags = METH_VARARGS;
    definition->ml_doc = doc.c_str();

    return definition;
}

PyObject* generatePyCallable(std::string moduleName, PyMethodDef* methodDefinition)
{
    PyObject* moduleNamePyString = PyUnicode_FromString(moduleName.c_str());
Unknown::Python::getInterpreter()->checkError(moduleNamePyString);
    PyObject* callable = PyCFunction_NewEx(methodDefinition, (PyObject*)NULL, moduleNamePyString);
    Unknown::Python::getInterpreter()->checkError(callable);

    return callable;
}

void createMethod(std::string moduleName, PyObject* callable, std::string functionName)
{
    PyObject* moduleNameString = PyUnicode_FromString(moduleName.c_str());
    PyObject* module = PyImport_Import(moduleNameString);
    Unknown::Python::getInterpreter()->checkError(module);
    PyObject* originalDict = PyModule_GetDict(module);
    Unknown::Python::getInterpreter()->checkError(originalDict);
    PyDict_SetItemString(originalDict, functionName.c_str(), callable);
}

void registerMethod(std::string moduleName, std::string functionName, std::string functionDescription, PyCFunction callback)
{
    PyMethodDef* methodDefinition = generatePyMethodDef(functionName, callback, functionDescription);
    PyObject* callable = generatePyCallable(moduleName, methodDefinition);
    createMethod(moduleName, callable, functionName);
}

PyObject* PyGetElement(PyObject* sequence, int index, std::string pyFuncName, int line) {
    PyObject* element = PySequence_GetItem(sequence, index);

    if(!element) {
        UK_LOG_ERROR_VERBOSE(::Unknown::concat("Invalid element at ", pyFuncName, ":", line));
        PyErr_PrintEx(1);
        return nullptr;
    }

    return element;
}

template <class T>
T* getObjectCapsule(PyObject* capsule, std::string name) {
    return (T*) PyCapsule_GetPointer(capsule, name.c_str());
}

#define MAKE_FUNC_MAPPING(name, desc, callback) registerMethod("Unknown", name, desc, callback);
#define PY_MAKE_FLOAT(value) PyFloat_FromDouble(value)
#define PY_MAKE_LONG(value) PyLong_FromLong(value)
#define PY_MAKE_CAPSULE(value, name, callback)  PyCapsule_New(value, name, callback)

#define PY_UNPACK_OBJ(T, name, args, index) getObjectCapsule<T>(PY_GET_OBJ(args, index), name)
#define PY_GET_OBJ(args, index) PyGetElement(args, index, __FUNCTION__, __LINE__)
#define PY_GET_LONG(args, index)  PyLong_AsLong(PY_GET_OBJ(args, index))
#define PY_GET_UTF8(args, index) PyUnicode_AsUTF8(PY_GET_OBJ(args, index))
#define PY_GET_FLOAT(args, index) PyFloat_AsDouble(PY_GET_OBJ(args, index))


PyObject* registerHookHandler(PyObject* self, PyObject* args)
{
    Unknown::HookType hookTypeEnum = (Unknown::HookType)PY_GET_LONG(args, 0);

    PyObject* callback = PY_GET_OBJ(args, 1);
    ::Unknown::registerHook([=]() {
        if(!PyObject_CallObject(callback, NULL)) {
            UK_LOG_ERROR("Unable to call func");
            PyObject_Print(callback, stdout, Py_PRINT_RAW);
            PyErr_PrintEx(1);
        }
    }, hookTypeEnum);
    Py_RETURN_NONE;
}

void rawImageDestructor(PyObject* imageCapsule)
{
    UK_LOG_INFO("Destroying image");
}

PyObject* createRawImageHandler(PyObject* self, PyObject* args)
{
    const char* fileName_cStr = PY_GET_UTF8(args, 0);
    printf("Loading file '%s'\n", fileName_cStr);

    std::unique_ptr<Unknown::Graphics::Image> image = UK_LOAD_IMAGE(fileName_cStr);
    PyObject* capsule = PY_MAKE_CAPSULE(image.release(), "Image", [](PyObject* a){});

    if(!capsule) {
        UK_LOG_ERROR(::Unknown::concat("Unable to form capsule@", __FILE__, "@", __FUNCTION__, ":", __LINE__));
        PyErr_PrintEx(1);
        return nullptr;
    }

    return capsule;
}

PyObject* renderRawImageHandler(PyObject* self, PyObject* args)
{
    PyObject* dataTuple = PY_GET_OBJ(args, 1);

    int XCoord = PY_GET_LONG(dataTuple, 0);
    int YCoord = PY_GET_LONG(dataTuple, 1);
    int angle = PY_GET_LONG(dataTuple, 2);
    PyObject* clipTuple = PyTuple_GetItem(dataTuple, 3);

    Unknown::Graphics::Image* image = PY_UNPACK_OBJ(Unknown::Graphics::Image, "Image", args, 0);

    if(image) {
        if(PyTuple_Check(clipTuple)) {
            // Only use user given clip if it is not None
            SDL_Rect clipRect;
            clipRect.x = PY_GET_LONG(clipTuple, 0);
            clipRect.y = PY_GET_LONG(clipTuple, 1);
            clipRect.w = PY_GET_LONG(clipTuple, 2);
            clipRect.h = PY_GET_LONG(clipTuple, 3);

            image->render(XCoord, YCoord, angle, &clipRect);
        } else {
            image->render(XCoord, YCoord, angle, NULL);
        }
    }

    Py_RETURN_NONE;
}

PyObject* createRawTimer(PyObject* self, PyObject* args)
{
    double timeStep = PY_GET_FLOAT(args, 0);
    Unknown::Timer* timer = new Unknown::Timer(timeStep);

    PyObject* capsule = PY_MAKE_CAPSULE(timer, "Timer", [](PyObject* a){});

    if(!capsule) {
        UK_LOG_ERROR("Unable to form capsule");
        PyErr_PrintEx(1);
        return nullptr;
    }

    return capsule;
}

PyObject* resetRawTimer(PyObject* self, PyObject* args)
{
    Unknown::Timer* timer = PY_UNPACK_OBJ(Unknown::Timer, "Timer", args, 0);

    if(timer) {
        timer->resetTimer();
    }

    Py_RETURN_NONE;
}

PyObject* checkRawTimer(PyObject* self, PyObject* args)
{
    Unknown::Timer* timer = PY_UNPACK_OBJ(Unknown::Timer, "Timer", args, 0);

    if(timer) {
        if(timer->isTickComplete()) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }

    }
}

PyObject* registerRawEventHandler(PyObject* self, PyObject* args)
{
    Unknown::EventType type = (Unknown::EventType)PY_GET_LONG(args, 0);

    const char* handlerName_cStr =  PY_GET_UTF8(args, 1);

    PyObject* callback = PY_GET_OBJ(args, 2);

    ::Unknown::registerEventHandler(type, handlerName_cStr, [=](Unknown::Event& evnt) {
        PyObject* args = PyTuple_New(3);
        PyTuple_SetItem(args, 0, PyLong_FromLong(evnt.SDLCode));
        PyTuple_SetItem(args, 1, PyLong_FromLong(evnt.keyCode));
        PyTuple_SetItem(args, 2, PyLong_FromLong(evnt.keyState));

        if(!PyObject_CallObject(callback, args)) {
            UK_LOG_ERROR("Unable to call func");
            PyObject_Print(callback, stdout, Py_PRINT_RAW);
            PyErr_PrintEx(1);
        }
    });

    Py_RETURN_NONE;
}

PyObject* logMessage(PyObject* self, PyObject* args)
{
    int loglevelValue = PY_GET_LONG(args, 0);

    const char* message_cStr = PY_GET_UTF8(args, 1);

    Unknown::log(loglevelValue, message_cStr);

    Py_RETURN_NONE;
}

PyObject* createRawSprite(PyObject* self, PyObject* args)
{
    int typeValue = PY_GET_LONG(args, 0);

    PyObject* data = PY_GET_OBJ(args, 1);

    // All sprites have an x and a y coord

    int xValue = PY_GET_LONG(data, 0);

    int yValue = PY_GET_LONG(data, 1);

    PyObject* capsule = nullptr;

    if(typeValue == 0) // If is a regular sprite
    {
        Unknown::Sprite *sprite = new Unknown::Sprite(xValue, yValue);

        capsule = PY_MAKE_CAPSULE(sprite, "Sprite", [](PyObject* a){});
    }

    if(typeValue == 1) // If is an image sprite
    {
        Unknown::Graphics::Image* imgValue = PY_UNPACK_OBJ(Unknown::Graphics::Image, "Image", data, 2);

        Unknown::Graphics::ImageSprite* sprite = new Unknown::Graphics::ImageSprite(xValue, yValue, imgValue);

        capsule = PY_MAKE_CAPSULE(sprite, "Sprite", [](PyObject* a){});
    }

    if(typeValue == 2) // If is an animated sprite
    {
        Unknown::Graphics::Animation* animationValue = PY_UNPACK_OBJ(Unknown::Graphics::Animation, "Animation", data, 2);

        Unknown::Graphics::AnimatedSprite *sprite = new Unknown::Graphics::AnimatedSprite(xValue, yValue, animationValue);

        capsule = PY_MAKE_CAPSULE(sprite, "Sprite", [](PyObject* a){});
    }

    if(!capsule) {
        UK_LOG_ERROR(::Unknown::concat("Unable to form capsule@", __FILE__, "@", __FUNCTION__, ":", __LINE__));
        PyErr_PrintEx(1);
        return nullptr;
    }

    return capsule;
}

PyObject* getMousePos(PyObject *self, PyObject *args) {
    Unknown::Point<int> p = Unknown::getMouseLocation();
    PyObject* seq = PyTuple_Pack(2, PyLong_FromLong(p.x), PyLong_FromLong(p.y));

    return seq;
}

PyObject* rawSpriteCall(PyObject* self, PyObject* args) {
    Unknown::Sprite* spr = PY_UNPACK_OBJ(Unknown::Sprite, "Sprite", args, 0);
    int functionID = PY_GET_LONG(args, 1);
    PyObject* data = PY_GET_OBJ(args, 2);

    switch (functionID)
    {
        case 0: spr->render();break;
        case 1: spr->init();break;
        case 2: spr->move(PY_GET_LONG(data, 0), PY_GET_LONG(data, 1));break;
        case 3:
            return PY_MAKE_CAPSULE(spr->clone(), "Sprite", [](PyObject* sprite) {
                Unknown::Sprite* s = getObjectCapsule<Unknown::Sprite>(sprite, "Sprite");
                // TODO:
                UK_LOG_INFO("Cleaned up sprite");
            });
        case 4:
            return PY_MAKE_LONG(spr->getAngle());
        case 5: spr->setAngle(PY_GET_FLOAT(data, 0));break;
        case 6: return PY_MAKE_CAPSULE(&spr->bounds, "Bounds", [](PyObject* bound){});
        case 7: return PY_MAKE_CAPSULE(&spr->direction, "Direction", [](PyObject* dir){});
        case 8: return PY_MAKE_CAPSULE(&spr->location, "Loc", [](PyObject* a){});
        case 9: spr->bounds = *PY_UNPACK_OBJ(Unknown::AABB, "Sprite", data, 0);break;
        case 10: spr->direction = *PY_UNPACK_OBJ(Unknown::Vector, "Vector", data, 0);break;
        case 11: spr->location = *PY_UNPACK_OBJ(Unknown::Point<double>, "Point", data, 0);break;

    }
    Py_RETURN_NONE;
}

PyObject* rawVectorInterface(PyObject* self, PyObject* args) {
    int functionID = PY_GET_LONG(args, 1);
    PyObject* data = PY_GET_OBJ(args, 2);

    if (functionID == 0) {
        return PY_MAKE_CAPSULE(new Unknown::Vector(PY_GET_FLOAT(data, 0), PY_GET_FLOAT(data, 1)), "Vector", [](PyObject* a){}); //TODO
    }

    Unknown::Vector* vector = PY_UNPACK_OBJ(Unknown::Vector, "Vector", args, 0);

    switch (functionID) {
        case 1:
        {
            return PY_MAKE_FLOAT(vector->getLength());
        }
        case 2:
        {
            UK_LOG_ERROR("Normalisation of vectors not impl");
            return PY_GET_OBJ(args, 0);
        }
        case 3:
        {
            PyObject* args_ = PyTuple_New(2);
            PyTuple_SetItem(args_, 0, PY_MAKE_FLOAT(vector->x));
            PyTuple_SetItem(args_, 1, PY_MAKE_FLOAT(vector->y));
            return args_;
        }
    }

    Py_RETURN_NONE;
}

void Unknown::Python::Interpreter::loadScript(std::string name)
{
    MAKE_FUNC_MAPPING("raw_vector_interface", "Call a function on a vector", rawVectorInterface);
    MAKE_FUNC_MAPPING("raw_sprite_interface", "Call a function on a sprite", rawSpriteCall);
    registerMethod("Unknown", "register_hook", "Add a base hook", registerHookHandler);
    registerMethod("Unknown", "create_raw_image", "Create an image capsule", createRawImageHandler);
    registerMethod("Unknown", "render_raw_image", "Render an image capsule", renderRawImageHandler);
    registerMethod("Unknown", "create_raw_timer", "Create a timer capsule", createRawTimer);
    registerMethod("Unknown", "timer_reset", "Resets a timer capsule", resetRawTimer);
    registerMethod("Unknown", "timer_isTickComplete", "Checks a timer capsule", checkRawTimer);
    registerMethod("Unknown", "event_register_handler", "Register a key handler", registerRawEventHandler);
    registerMethod("Unknown", "uk_log", "Print a string to stdout", logMessage);
    registerMethod("Unknown", "create_raw_sprite", "Create a sprite capsule", createRawSprite);
    registerMethod("Unknown", "get_mouse_pos", "Get mouse position", getMousePos);

    log(UK_LOG_LEVEL_INFO, concat("Loading script", name));

    PyObject *testModule = PyImport_ImportModule(name.c_str());
    checkError(testModule);
    if (PyObject_HasAttrString(testModule, "init"))
    {
        PyObject *initFunction = PyObject_GetAttrString(testModule, "init");
        if (initFunction)
        {
            PyObject_CallObject(initFunction, NULL);
        }
    }
}