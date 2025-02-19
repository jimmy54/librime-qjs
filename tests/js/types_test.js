function checkArgument(arg) {
  assert(arg.namespace === 'namespace')
  assert(arg.candidate.text === 'text')
  assert(arg.engine.schema.id === '.default')
  arg.candidate.text = 'new text'

  const config = arg.engine.schema.config
  assert(config.getBool('key') === null)
  assert(config.getBool('key1') === true)
  assert(config.getBool('key2') === false)
  assert(config.getInt('key3') === 666)
  assert(config.getDouble('key4') === 0.999)
  assert(config.getString('key5') === 'string')

  const list = config.getList('list')
  assert(list.getValueAt(0).getString() === 'item1')
  assert(list.getValueAt(1).getString() === 'item2')
  assert(list.getValueAt(2).getString() === 'item3')
  assert(list.getValueAt(3) === null)

  config.setString('greet', 'hello from js')

  const context = arg.engine.context
  assert(context.input === 'hello')

  context.input = 'world'

  assert(context.preedit !== null)
  console.log(context.preedit.text) // => [nothing]
  assert(context.preedit.text?.length === 5)
  assert(context.preedit.caretPos === 5)
  assert(context.preedit.selectStart === 0)
  assert(context.preedit.selectEnd === 0)

  arg.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)

  return arg
}

function assert(condition, msg) {
  if (!condition) {
    throw new Error('assertion failed: ' + msg)
  }
}

globalThis.checkArgument = checkArgument
