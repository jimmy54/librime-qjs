export class FilterWithIsApplicable {
  isApplicable(env) {
    console.log('filter_test isApplicable')
    return false
  }
  filter(candidates, env) {
    throw new Error('should not be called as isApplicable returns false')
  }
}
