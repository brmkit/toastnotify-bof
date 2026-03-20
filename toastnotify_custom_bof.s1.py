import base64
from typing import Dict, List, Optional, Tuple

from outflank_stage1.task.base_bof_task import BaseBOFTask
from outflank_stage1.task.enums import BOFArgumentEncoding
from outflank_stage1.task.exceptions import TaskInvalidArgumentsException


class ToastCustomBOF(BaseBOFTask):
    """Send a custom toast notification from an uploaded XML file."""

    def __init__(self):
        super().__init__("toast-custom", base_binary_name="toastnotify")

        self.parser.description = (
            "Send a custom Windows toast notification from an XML file. "
            "Upload a toast XML file via the GUI. Supports full template "
            "features: actions, protocol links, images, progress bars, etc."
        )

        self.parser.add_argument("aumid", help="Application User Model ID to impersonate")

        self.parser.epilog = (
            "Example usage:\n"
            "  toast-custom MSEdge    (then upload XML file via GUI)\n"
            "  toast-custom Windows.Defender.SecurityCenter\n"
            "\n"
            "Notes:\n"
            "  - Upload a toast XML file using the file upload GUI element.\n"
            "  - The XML is automatically base64-encoded before sending to the BOF.\n"
            "  - Use the Notifications Visualizer app to design complex templates.\n"
        )

    def validate_files(self, arguments: List[str]):
        xml_file = self.get_file_by_name("xml_file")
        if xml_file is None:
            raise TaskInvalidArgumentsException(
                "No XML file uploaded. Please upload a toast XML file."
            )

    def _encode_arguments_bof(
        self, arguments: List[str]
    ) -> List[Tuple[BOFArgumentEncoding, str]]:
        parser_arguments = self.parser.parse_args(arguments)
        aumid = parser_arguments.aumid.strip('"').strip("'")

        xml_file = self.get_file_by_name("xml_file")
        xml_content = xml_file.content
        if isinstance(xml_content, bytes):
            xml_content = xml_content.decode("utf-8")
        b64xml = base64.b64encode(xml_content.strip().encode("utf-8")).decode("ascii")

        # BOF expects: zzz (cmd, aumid, b64xml)
        return [
            (BOFArgumentEncoding.STR, "custom"),
            (BOFArgumentEncoding.STR, aumid),
            (BOFArgumentEncoding.STR, b64xml),
        ]

    def run(self, arguments: List[str]):
        xml_file = self.get_file_by_name("xml_file")
        uploaded_name = getattr(xml_file, "original_name", "uploaded XML file")
        self.append_response(
            f"Sending custom toast using XML from {uploaded_name}...\n"
        )
        super().run(arguments)

    def get_gui_elements(self) -> Optional[Dict]:
        return {
            "title": "Custom Toast Notification",
            "desc": "Send a custom toast notification from an XML template file.",
            "elements": [
                {
                    "name": "xml_file",
                    "type": "file",
                    "description": "Toast XML template file",
                    "placeholder": "Select XML file",
                },
            ],
        }
