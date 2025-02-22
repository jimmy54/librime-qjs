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
  console.log(context.preedit.text) // => x� with llvm clang, [nothing visible] with apple clang
  assert(context.preedit.text?.length >= 4)// => 4 with llvm clang, 5 with apple clang

  assert(context.preedit.caretPos > 0) // => 4515895616 with llvm clang, 5 with apple clang
  assert(!isNaN(context.preedit.selectStart)) // => 8 with llvm clang, 0 with apple clang
  assert(!isNaN(context.preedit.selectEnd)) // 140701946852032 with llvm clang, 0 with apple clang

  arg.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)

  testTrie()
  return arg
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
