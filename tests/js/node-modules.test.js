// test in commandline: `./qjs ./node-modules.test.js`

console.log(`log from node-modules.test.js`)

import { addDays, format } from 'date-fns'
const today = new Date(2025, 2, 17) // month starts from 0
assertEquals(format(today, 'yyyy-MM-dd'), '2025-03-17', 'should get correct date')
const tomorrow = addDays(today, 1)
assertEquals(format(tomorrow, 'yyyy-MM-dd'), '2025-03-18','should get correct tomorrow')


import { Solar } from 'lunar-typescript'
import { assertEquals } from './testutils'

const dt = Solar.fromYmdHms(2025, 3, 17, 11, 20, 0) // month starts from 1
console.log(dt.toFullString())
assertEquals(dt.toFullString(), '2025-03-17 11:20:00 星期一 双鱼座', 'should get correct date time')

const expectedLunarText =
  `二〇二五年二月十八 乙巳(蛇)年 己卯(兔)月 乙酉(鸡)日 午(马)时 纳音[覆灯火 城头土 泉中水 杨柳木] 星期一 ` +
  `北方玄武 星宿[危月燕](凶) 彭祖百忌[乙不栽植千株不长 酉不会客醉坐颠狂] ` +
  `喜神方位[乾](西北) 阳贵神方位[坤](西南) 阴贵神方位[坎](正北) 福神方位[坤](西南) 财神方位[艮](东北) 冲[(己卯)兔] 煞[东]`
console.log(dt.getLunar().toFullString())
assertEquals(dt.getLunar().toFullString(), expectedLunarText, 'should get correct lunar text')

// to bundle it to IIFE format to run in JavaScriptCore
export class DummyClass {}
