;(() => {
  var LazyFilterWithGenerator = class {
    *filter(iter, env) {
      let idx = 0
      do {
        let candidate = iter.next
        if (!candidate) {
          return
        }
        console.log(`checking candidate at index = ${idx}`)
        if (idx % 2 === 0) {
          yield candidate
        }
        idx++
      } while (true)
    }
  }
  globalThis.iife_instance_lazy_filter_iife_js = new LazyFilterWithGenerator()
})()
