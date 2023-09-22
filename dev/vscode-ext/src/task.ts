import { WorkspaceFolder } from 'vscode';
import { 
	ExtensionContext, 
	ShellExecution, 
	Task, 
	TaskDefinition, 
	TaskScope, 
	tasks, 
	workspace, 
	window 
} from 'vscode';

interface LobsterRunTaskDefinition extends TaskDefinition {
	main: string;
	lobsterArgs?: string[];
	programArgs?: string[];
}

export function registerTasks(context: ExtensionContext) {
	tasks.registerTaskProvider('lobster', {
		provideTasks: () => {
			return [makeTask(
				'Run current file', 
				{ 
					type: 'lobster',
					main: window.activeTextEditor.document.uri.fsPath 
				}, 
				TaskScope.Workspace)];
		},
		resolveTask(task: Task): Task | undefined {
			const definition: LobsterRunTaskDefinition = <any>task.definition;

			return makeTask(
				task.name,
				definition,
				task.scope || TaskScope.Workspace
			);
		}
	});
}

function makeTask(
	name: string, 
	definition: LobsterRunTaskDefinition, 
	scope: TaskScope | WorkspaceFolder
): Task {
	const config = workspace.getConfiguration('lobster', null);
	const executable = config.get<string>('executable');
	const imports = config.get<string[]>('imports');

	return new Task(
		definition,
		scope,
		name,
		'lobster',
		new ShellExecution(`${executable} ${imports?.map(i => "--import " + i).join(" ")}` +
			(definition.lobsterArgs?.join(' ') || '') +
			" " + definition.main +
			" -- " + (definition.programArgs?.join(' ') || '')
		)
	);
}