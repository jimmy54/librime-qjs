var __defProp = Object.defineProperty
var __name = (target, value) => __defProp(target, 'name', { value, configurable: true })
var accents =
  '\u0101\xE1\u01CE\xE0\u0113\xE9\u011B\xE8\u012B\xED\u01D0\xEC\u014D\xF3\u01D2\xF2\u016B\xFA\u01D4\xF9\u01D6\u01D8\u01DA\u01DC\xFC'
var without = 'aaaaeeeeiiiioooouuuuvvvvv'
var dict = {}
accents.split('').forEach((char, idx) => (dict[char] = without[idx]))
function unaccent(str) {
  return str
    .split('')
    .map((char) => {
      return dict[char] || char
    })
    .join('')
}
__name(unaccent, 'unaccent')
var _SortCandidatesByPinyinFilter = class _SortCandidatesByPinyinFilter {
  constructor(env) {
    console.log('sort_by_pinyin.js init')
  }
  finalizer() {
    console.log('sort_by_pinyin.js finit')
  }
  filter(candidates, env) {
    const userPhrases = []
    const userPhrasesIndices = []
    const candidatesWithPinyin = []
    const candidatesWithPinyinIndices = []
    const input = env.engine.context.input.replace(/\/.*$/, '')
    const size = candidates.length > MAX_SIZE_TO_SORT ? MAX_SIZE_TO_SORT : candidates.length
    candidates.slice(0, size).forEach((candidate, idx) => {
      var _a
      const pinyin = (_a = extractPinyin(candidate.comment)) == null ? void 0 : _a.replaceAll(' ', '')
      if (candidate.type === 'user_phrase') {
        const weight = getWeightByPinyin(pinyin, input, true) + size - idx
        userPhrasesIndices.push(idx)
        userPhrases.push({ candidate, weight })
      } else if (pinyin) {
        const weight = getWeightByPinyin(pinyin, input, false) + size - idx
        candidatesWithPinyinIndices.push(idx)
        candidatesWithPinyin.push({ candidate, weight })
      }
    })
    userPhrases.sort((a, b) => b.weight - a.weight)
    userPhrasesIndices.forEach((originalIndex, idx) => {
      candidates[originalIndex] = userPhrases[idx].candidate
    })
    candidatesWithPinyin.sort((a, b) => b.weight - a.weight)
    candidatesWithPinyinIndices.forEach((originalIndex, idx) => {
      candidates[originalIndex] = candidatesWithPinyin[idx].candidate
    })
    console.log('sort_by_pinyin.js filter', input, candidates.length)
    return candidates
  }
}
__name(_SortCandidatesByPinyinFilter, 'SortCandidatesByPinyinFilter')
var SortCandidatesByPinyinFilter = _SortCandidatesByPinyinFilter
var MAX_SIZE_TO_SORT = 100
function extractPinyin(comment) {
  const match = comment.match(/〖(.+?)〗/)
  if (match) {
    return unaccent(match[1])
  }
  const match2 = comment.match(/［(.*?)］/) || []
  return match2[1]
}
__name(extractPinyin, 'extractPinyin')
function getWeightByPinyin(pinyin, input, isInUserPhrase) {
  if (pinyin === input) {
    return 1e4
  }
  if (isInUserPhrase && !pinyin) {
    return 1e4
  }
  if (pinyin == null ? void 0 : pinyin.startsWith(input)) {
    return 5e3
  }
  if (pinyin == null ? void 0 : pinyin.includes(input)) {
    return 1e3 + pinyin.length
  }
  return 0
}
__name(getWeightByPinyin, 'getWeightByPinyin')
export { SortCandidatesByPinyinFilter }
