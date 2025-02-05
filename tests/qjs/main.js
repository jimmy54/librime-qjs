import { MyClass } from "./lib.js";

// JSValue myClass = JS_GetPropertyStr(ctx, global_obj, "MyClass");
globalThis.MyClass = MyClass; // necessary to get the constructor in c++

// test in commandline: `./qjs ./main.js`
const obj = new MyClass(10);
console.log(obj.greet("QuickJS"));
console.log(obj.myMethod());
