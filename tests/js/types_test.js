function checkArgument(arg) {
    assert(arg.namespace === 'namespace')
    assert(arg.candidate.text === 'text')
    assert(arg.engine.schema.id === '.default')

    const config = arg.engine.schema.config
    assert(config.getBool("key") === null)
    assert(config.getBool("key1") === true)
    assert(config.getBool("key2") === false)
    assert(config.getInt("key3") === 666)
    assert(config.getDouble("key4") === 0.999)
    assert(config.getString("key5") === 'string')

    config.setString("greet", 'hello from js')

    const context = arg.engine.context
    assert(context.input === 'hello')
    context.input = 'world'
    
    return arg
}

function assert(condition, msg) {
    if (!condition) {
        throw new Error("assertion failed: " + msg)
    }
}

globalThis.checkArgument    = checkArgument
