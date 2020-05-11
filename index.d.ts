declare namespace duktype
{
    export class Context
    {
        constructor();
        getGlobalObject(): Scope;
        eval(script: string): any;
        enablePromises(): void;
        enableTimers(): void;
    }

    export class Scope
    {
        setProperty(name: string, value: any): void;
        getProperty(name: string): any;
        deleteProperty(name: string): void;
        callMethod(name: string, ...args: any[]): any;
        createObject(name: string): Scope;
    }

    export class DukCallback
    {
        call(...args: any[]): any;
    }
}

export = duktype;