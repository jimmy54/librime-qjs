// test in commandline: `./qjs ./main.js`
import * as std from 'qjs:std'
import { MyClass } from './lib.js'

globalThis.MyClass = MyClass // necessary to get the constructor in c++

const obj = new MyClass(10)
console.log(obj.greet('QuickJS'))
console.log(obj.myMethod())

console.log(obj.greet?.name.includes('greet'))
console.log(obj.hello?.name.includes('greet'))

// test load file with utf-8 chars
const content = std.loadFile('./lib.js')
console.log('File contents => ', content)
