import { MyClass } from "./lib.js";

globalThis.funcWithRuntimeError = function () {
    // Possibly unhandled promise rejection: ReferenceError: abcdefg is not defined
    // at <anonymous> (./runtime-error.js:7:21)
    // at <anonymous> (./runtime-error.js:11:1)
    const obj = new MyClass(abcdefg);
    obj.hi();
}

funcWithRuntimeError();
