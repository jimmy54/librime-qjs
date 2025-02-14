export function init() {
    console.log('filter_test init')
}
export function finit() {
    console.log('filter_test finit')
}
export function filter(candidates) {
    console.log('filter_test filter')
    return candidates.filter(it => it.text === 'text1')
}
