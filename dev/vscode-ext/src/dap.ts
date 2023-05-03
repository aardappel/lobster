import { 
    DebugAdapterDescriptor, 
    DebugAdapterDescriptorFactory,
    DebugAdapterExecutable,
    DebugSession, 
    ProviderResult, 
    DebugAdapterInlineImplementation 
} from 'vscode';

export class InlineLobsterDebugAdapterFactory implements DebugAdapterDescriptorFactory {

    constructor(readonly module: string) {}

    createDebugAdapterDescriptor(
        session: DebugSession, 
        executable: DebugAdapterExecutable
    ): ProviderResult<DebugAdapterDescriptor> {
        // eslint-disable-next-line @typescript-eslint/no-var-requires
        return new DebugAdapterInlineImplementation(require(this.module));
    }

}