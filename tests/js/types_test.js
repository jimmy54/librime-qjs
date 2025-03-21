import { assert, assertEquals } from "./testutils"

function checkArgument(env) {
  assertEquals(env.namespace, 'namespace')
  assertEquals(env.candidate.text, 'text')
  assertEquals(env.engine.schema.id, '.default')
  env.candidate.text = 'new text'

  const config = env.engine.schema.config
  assertEquals(config.getBool('key'), null)
  assertEquals(config.getBool('key1'), true)
  assertEquals(config.getBool('key2'), false)
  assertEquals(config.getInt('key3'), 666)
  assertEquals(config.getDouble('key4'), 0.999)
  assertEquals(config.getString('key5'), 'string')

  const list = config.getList('list')
  assertEquals(list.getValueAt(0).getString(), 'item1')
  assertEquals(list.getValueAt(1).getString(), 'item2')
  assertEquals(list.getValueAt(2).getString(), 'item3')
  assertEquals(list.getValueAt(3), null)

  config.setString('greet', 'hello from js')

  const context = env.engine.context
  assertEquals(context.input, 'hello')

  assert(context.preedit !== null, 'preedit should not be null')
  assertEquals(context.preedit.text, 'hello', 'preedit should have text')
  assertEquals(context.preedit.caretPos, 5, 'preedit should have caretPos')
  assertEquals(context.preedit.selectStart, 0, 'preedit should have selectStart')
  assertEquals(context.preedit.selectEnd, 0, 'preedit should have selectEnd')

  context.input = 'world'

  env.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)

  // ensure adding extra fields to the qjs object would not break the quickjs engine
  env.newCandidate.extraField = 'extra field'
  assertEquals(env.newCandidate.extraField, 'extra field')

  testEnvUtilities(env)
  testTrie(env)

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

  assertEquals(env.popen(`echo libRime-qjs`).trim(), 'libRime-qjs')

  assertEquals(env.fileExists(env.currentFolder + '/js/types_test.js'), true)
  assertEquals(env.fileExists(env.currentFolder + '/js/not_found.js'), false)

  // test load file with utf-8 chars
  const content = env.loadFile(env.currentFolder + '/js/types_test.js')
  assert(content.includes('测试 UTF-8 编码'))
}

function testTrie(env) {
  const trie = new Trie()
  trie.loadTextFile(env.currentFolder + '/dummy_dict.txt', 6)
  checkTrieData(trie)

  trie.saveToBinaryFile(env.currentFolder + '/dumm.bin')
  const trie2 = new Trie()
  trie2.loadBinaryFile(env.currentFolder + '/dumm.bin')
  checkTrieData(trie2)
}

function checkTrieData(trie) {
  const result1 = trie.find('accord')
  assertEquals(result1, '[ә\'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, 使一致\\n vi. 相符合')
  const result2 = trie.find('accordion')
  assertEquals(result2, '[ә\'kɒ:djәn]; n. 手风琴\\n a. 可折叠的')
  const result3 = trie.find('nonexistent-word')
  assertEquals(result3, null)
  const prefix_results = trie.prefixSearch('accord')
  assertEquals(prefix_results.length, 6)
}


globalThis.checkArgument = checkArgument

const load_file_test_data = [
  'Hello, 世界!',
  '测试 UTF-8 编码',
  '🌟 Emoji test',
  'Mixed content: あいうえお',
]
