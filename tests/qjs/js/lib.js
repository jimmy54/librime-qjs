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
