export function init(env) {
  console.log('filter_test init')
  assert(env.namespace === 'filter_test')
  assert(env.userDataDir === 'tests/js')
  const config = env.engine.schema.config
  assert(config.getString('greet') === 'hello from c++')

  // test load file with utf-8 chars
  const content = env.loadFile('tests/js/types_test.js')
  assert(content.includes('测试 UTF-8 编码'))
}
export function finit(env) {
  console.log('filter_test finit')
  assert(env.namespace === 'filter_test')
  const config = env.engine.schema.config
  assert(config.getString('greet') === 'hello from c++')
}
export function filter(candidates, env) {
  console.log('filter_test filter', candidates.length)
  assert(env.namespace === 'filter_test')
  const config = env.engine.schema.config
  assert(config.getString('greet') === 'hello from c++')

  const expectingText = config.getString('expectingText')
  assert(expectingText === 'text2')
  return candidates.filter((it) => it.text === expectingText)
}

function assert(condition, msg) {
  if (!condition) {
    throw new Error('assertion failed: ' + msg)
  }
}
