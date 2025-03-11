export class TestFilter {
  constructor(env) {
    console.log('filter_test init')
    assert(env.namespace === 'filter_test')
    assert(env.userDataDir === 'tests/js')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')
  }
  finalizer(env) {
    console.log('filter_test finit')
    assert(env.namespace === 'filter_test')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')
  }
  filter(candidates, env) {
    console.log('filter_test filter', candidates.length)
    assert(env.namespace === 'filter_test')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')

    const expectingText = config.getString('expectingText')
    assert(expectingText === 'text2')
    return candidates.filter((it) => it.text === expectingText)
  }
}

function assert(condition, msg) {
  if (!condition) {
    throw new Error('assertion failed: ' + msg)
  }
}
