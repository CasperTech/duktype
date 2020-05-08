import * as duktype from '../index'

{
    const ctx = new duktype.Context();

    try
    {
        const scope = ctx.getGlobalObject();
        const foo = scope.createObject('Foo');
        const bar = foo.createObject('Bar');
        scope.setProperty('print', (str: string) =>
        {
            console.log('JS Print: ' + str);
            return true;
        });
        foo.setProperty('testFoo', (num: number) =>
        {
            console.log('Called foo with ' + num);
            return true;
        });
        bar.setProperty('testBar', () =>
        {
            console.log('Called bar');
            return true;
        });

        foo.setProperty('name', 'Fred');

        const prop = foo.getProperty('name');
        console.log('Got property: ' + prop);

        bar.setProperty('name', 'Jobs');

        scope.setProperty('getName', (b: any) =>
        {
            if (!b.name)
            {
                return 'No name';
            }
            return b.name;
        });

        const result1 = foo.callMethod('testFoo', 1);
        console.log('Result1: ' + result1);
        const result2 = bar.callMethod('testBar');
        console.log('Result2: ' + result1);
        const name = ctx.eval('getName(Foo);');
        console.log('Name is ' + name);
        const name2 = ctx.eval('getName(Foo.Bar);');
        console.log('Name is ' + name2);

        bar.deleteProperty('name');
        const name3 = ctx.eval('getName(Foo.Bar);');
        console.log('Name is now: ' + name3);

    }
    catch (error)
    {
        console.log(error.stack);
    }
}