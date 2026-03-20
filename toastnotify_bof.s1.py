import base64
from typing import List, Tuple, Optional, Dict

from outflank_stage1.task.base_bof_task import BaseBOFTask
from outflank_stage1.task.enums import BOFArgumentEncoding
from outflank_stage1.task.exceptions import TaskInvalidArgumentsException


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


class ToastCustomBOF(BaseBOFTask):
    """Send a custom toast notification from XML."""

    def __init__(self):
        super().__init__("toast-custom", base_binary_name="toastnotify")

        self.parser.description = (
            "Send a custom Windows toast notification from an XML payload. "
            "Accepts either raw XML (auto-encoded to base64) or pre-encoded "
            "base64 XML via the --b64 flag. Supports full toast template "
            "features: actions, protocol links, images, progress bars, etc."
        )

        self.parser.add_argument("aumid", help="Application User Model ID to impersonate")
        self.parser.add_argument("xml", help="Toast XML payload (raw XML or base64 with --b64)")
        self.parser.add_argument(
            "--b64",
            action="store_true",
            help="Treat xml argument as already base64-encoded",
        )

        self.parser.epilog = (
            "Example usage:\n"
            '  toast-custom MSEdge \'<toast><visual><binding template="ToastGeneric">'
            "<text>Hello</text></binding></visual></toast>'\n"
            "  toast-custom MSEdge PHRvYXN0P... --b64\n"
            "\n"
            "Notes:\n"
            "  - By default, raw XML is automatically base64-encoded before sending.\n"
            "  - Use --b64 if you have already encoded the XML payload.\n"
            "  - Use the Notifications Visualizer app to design complex templates.\n"
        )

    def validate_arguments(self, arguments: List[str]):
        super().validate_arguments(arguments)

        parser_arguments = self.parser.parse_args(arguments)

        if parser_arguments.b64:
            try:
                base64.b64decode(parser_arguments.xml, validate=True)
            except Exception:
                raise TaskInvalidArgumentsException(
                    "Invalid base64 payload. Ensure the XML is properly base64-encoded."
                )

    def _encode_arguments_bof(
        self, arguments: List[str]
    ) -> List[Tuple[BOFArgumentEncoding, str]]:
        parser_arguments = self.parser.parse_args(arguments)

        if parser_arguments.b64:
            b64xml = parser_arguments.xml
        else:
            b64xml = base64.b64encode(parser_arguments.xml.encode("utf-8")).decode("ascii")

        # BOF expects: zzz (cmd, aumid, b64xml)
        aumid = parser_arguments.aumid.strip('"').strip("'")

        return [
            (BOFArgumentEncoding.STR, "custom"),
            (BOFArgumentEncoding.STR, aumid),
            (BOFArgumentEncoding.STR, b64xml),
        ]
