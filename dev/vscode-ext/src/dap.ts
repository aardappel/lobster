import { 
    DebugAdapterDescriptor, 
    DebugAdapterDescriptorFactory,
    DebugAdapterExecutable,
    DebugSession, 
    ProviderResult, 
    DebugAdapterInlineImplementation 
} from 'vscode';

//TODO split DAP from VSCode Extension, so that other IDEs can use it.
import { LobsterDebugSession } from './lobsterDebug';

export class InlineLobsterDebugAdapterFactory implements DebugAdapterDescriptorFactory {
    createDebugAdapterDescriptor(
        session: DebugSession, 
        executable: DebugAdapterExecutable
    ): ProviderResult<DebugAdapterDescriptor> {
        return new DebugAdapterInlineImplementation(new LobsterDebugSession());
    }
}