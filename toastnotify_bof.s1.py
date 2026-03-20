from typing import List, Tuple

from outflank_stage1.task.base_bof_task import BaseBOFTask
from outflank_stage1.task.enums import BOFArgumentEncoding


class ToastGetAumidBOF(BaseBOFTask):
    """Enumerate registered AUMIDs on the target system."""

    def __init__(self):
        super().__init__("toast-getaumid", base_binary_name="toastnotify")

        self.parser.description = (
            "Enumerate Application User Model IDs (AUMIDs) registered in "
            "HKCU and HKLM notification settings. Use these IDs with "
            "toast-sendtoast or toast-custom to spoof notification sources."
        )

        self.parser.epilog = (
            "Example usage:\n"
            "  toast-getaumid\n"
        )

    def _encode_arguments_bof(
        self, arguments: List[str]
    ) -> List[Tuple[BOFArgumentEncoding, str]]:
        # BOF expects: z (command string only)
        return [
            (BOFArgumentEncoding.STR, "getaumid"),
        ]


class ToastSendtoastBOF(BaseBOFTask):
    """Send a simple toast notification with a title and body."""

    def __init__(self):
        super().__init__("toast-sendtoast", base_binary_name="toastnotify")

        self.parser.description = (
            "Send a Windows toast notification with a title and body text. "
            "Builds ToastGeneric XML internally. Requires a valid AUMID "
            "(use toast-getaumid to enumerate available IDs)."
        )

        self.parser.add_argument("aumid", help="Application User Model ID to impersonate")
        self.parser.add_argument("title", help="Notification title text")
        self.parser.add_argument("body", help="Notification body text")

        self.parser.epilog = (
            "Example usage:\n"
            '  toast-sendtoast MSEdge "Update Available" "Click to install the latest update."\n'
            '  toast-sendtoast com.squirrel.AnthropicClaude.claude "Sync Complete" "All files synced."\n'
        )

    def _encode_arguments_bof(
        self, arguments: List[str]
    ) -> List[Tuple[BOFArgumentEncoding, str]]:
        parser_arguments = self.parser.parse_args(arguments)

        # BOF expects: zzzz (cmd, aumid, title, text)
        # Strip surrounding quotes that OC2 may preserve from the command line
        aumid = parser_arguments.aumid.strip('"').strip("'")
        title = parser_arguments.title.strip('"').strip("'")
        body = parser_arguments.body.strip('"').strip("'")

        return [
            (BOFArgumentEncoding.STR, "sendtoast"),
            (BOFArgumentEncoding.STR, aumid),
            (BOFArgumentEncoding.STR, title),
            (BOFArgumentEncoding.STR, body),
        ]
