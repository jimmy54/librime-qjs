export function init(env) {
  console.log('[processor_test] init')
  const config = env.engine.schema.config
  const initTestValue = config.getString('init_test')
  if (initTestValue) {
    console.log(`[processor_test] init_test value: ${initTestValue}`)
  }
  return true
}

export function finit(env) {
  console.log('[processor_test] finit')
  return true
}

export function process(keyEvent, env) {
  assertEquals(env.engine.context.lastSegment?.prompt, 'prompt', 'should have lastSegment with prompt')

  const repr = keyEvent.repr
  if (repr === 'space') {
    return 'kAccepted'
  } else if (repr === 'Return') {
    return 'kRejected'
  }

  return 'kNoop'
}

function assertEquals(actual, expected, message) {
  const actualStr = JSON.stringify(actual)
  const expectedStr = JSON.stringify(expected)
  if (actualStr !== expectedStr) {
    throw new Error(`Expected ${expectedStr}, but got ${actualStr}. <== ${message}`)
  }
}
