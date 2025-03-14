function checkArgument(env) {
  assert(env.namespace === 'namespace')
  assert(env.candidate.text === 'text')
  assert(env.engine.schema.id === '.default')
  env.candidate.text = 'new text'

  const config = env.engine.schema.config
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

  const context = env.engine.context
  assert(context.input === 'hello')

  assert(context.preedit !== null, 'preedit should not be null')
  assert(context.preedit.text === 'hello', 'preedit should have text')
  assert(context.preedit.caretPos === 5, 'preedit should have caretPos')
  assert(context.preedit.selectStart === 0, 'preedit should have selectStart')
  assert(context.preedit.selectEnd === 0, 'preedit should have selectEnd')

  context.input = 'world'

  env.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)

  // ensure adding extra fields to the qjs object would not break the quickjs engine
  env.newCandidate.extraField = 'extra field'
  assert(env.newCandidate.extraField === 'extra field')

  testEnvUtilities(env)
  testTrie()

  return env
}

function testEnvUtilities(env) {
  const info = env.getRimeInfo()
  console.log(`Rime info = ${info}`)
  assert(info.includes('libRime v'))
  assert(info.includes('libRime-qjs v'))
  assert(info.includes('Process RSS Mem: '))
  assert(info.includes('QuickJS Mem: '))

  console.error(`This is an error message.`)

  // ensure engine.processKey would not crash the program
  env.engine.processKey('Down')
  env.engine.processKey('InvalidKey')

  assertEquals(env.popen(`echo 'libRime-qjs'`).trim(), 'libRime-qjs')

  assertEquals(env.fileExists('tests/js/types_test.js'), true)
  assertEquals(env.fileExists('tests/js/not_found.js'), false)

  // test load file with utf-8 chars
  const content = env.loadFile('tests/js/types_test.js')
  assert(content.includes('测试 UTF-8 编码'))
}

function testTrie() {
  const trie = new Trie()
  trie.loadTextFile('./tests/dummy_dict.txt', 6)
  checkTrieData(trie)

  trie.saveToBinaryFile('./tests/dumm.bin')
  const trie2 = new Trie()
  trie2.loadBinaryFile('./tests/dumm.bin')
  checkTrieData(trie2)
}

function checkTrieData(trie) {
  const result1 = trie.find('accord')
  assert(result1 === '[ә\'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, 使一致\\n vi. 相符合')
  const result2 = trie.find('accordion')
  assert(result2 === '[ә\'kɒ:djәn]; n. 手风琴\\n a. 可折叠的')
  const result3 = trie.find('nonexistent-word')
  assert(result3 === null)
  const prefix_results = trie.prefixSearch('accord')
  assert(prefix_results.length === 6)
}

function assertEquals(actual, expected, message) {
  const actualStr = JSON.stringify(actual)
  const expectedStr = JSON.stringify(expected)
  if (actualStr !== expectedStr) {
    throw new Error(`Expected ${expectedStr}, but got ${actualStr}. ${message}`)
  }
}


function assert(condition, msg) {
  if (!condition) {
    throw new Error('assertion failed: ' + msg)
  }
}

globalThis.checkArgument = checkArgument

const load_file_test_data = [
  'Hello, 世界!',
  '测试 UTF-8 编码',
  '🌟 Emoji test',
  'Mixed content: あいうえお',
]
