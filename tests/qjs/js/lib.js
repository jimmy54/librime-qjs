export function greet(name) {
    return `Hello ${name}!`;
}

export class MyClass {
  constructor(value) {
    this.value = value;
  }

  myMethod() {
    return this.value + 1;
  }
  greet(name) {
    return greet(name);
  }
}

const quickjs_load_file_test_data = [
  'Hello, 世界!',
  '测试 UTF-8 编码',
  '🌟 Emoji test',
  'Mixed content: あいうえお',
]
