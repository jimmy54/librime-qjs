// test in commandline: `./qjs ./main.js`
import { MyClass } from './lib.js'

globalThis.MyClass = MyClass // necessary to get the constructor in c++

const obj = new MyClass(10)
console.log(obj.greet('QuickJS'))
console.log(obj.myMethod())

console.log(obj.greet?.name.includes('greet'))
console.log(obj.hello?.name.includes('greet'))
