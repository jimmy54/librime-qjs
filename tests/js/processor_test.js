import { assertEquals } from './testutils.js'

export class TestProcessor {
  constructor(env) {
    console.log('[processor_test] init')
    const config = env.engine.schema.config
    const initTestValue = config.getString('init_test')
    if (initTestValue) {
      console.log(`[processor_test] init_test value: ${initTestValue}`)
    }
  }

  finalizer() {
    console.log('[processor_test] finit')
  }

  process(keyEvent, env) {
    assertEquals(env.engine.context.lastSegment?.prompt, 'prompt', 'should have lastSegment with prompt')

    const repr = keyEvent.repr
    if (repr === 'space') {
      return 'kAccepted'
    } else if (repr === 'Return') {
      return 'kRejected'
    }

    return 'kNoop'
  }
}
