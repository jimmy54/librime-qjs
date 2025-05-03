export class FastFilterWithGenerator {
  *filter(iter, env) {
    for (let idx = 0, candidate; (candidate = iter.next()); ++idx) {
      console.log(`checking candidate at index = ${idx}`)
      if (idx % 2 === 0) {
        yield candidate
      }
    }
  }
}
