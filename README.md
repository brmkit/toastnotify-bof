# toastnotify-bof

A Beacon Object File (BOF) for sending Windows toast notifications. Pairs with the [blog post](https://todo) for full context and use cases.

## Subcommands

### getaumid

Enumerates AUMIDs (Application User Model IDs) registered on the system. Use this to find a suitable identity to borrow before sending a toast.

```
inline-execute toastnotify.o go getaumid
```

Example output:

```
[Notifications\Settings - HKCU]
  Microsoft.Windows.Explorer
  MSEdge
  com.squirrel.AnthropicClaude.claude
  ...
[Notifications\Settings - HKLM]
  ...
  com.squirrel.AnthropicClaude.claude
  ...
```

### sendtoast

Sends a standard toast notification with a title and body text. Builds the XML internally using `ToastGeneric`.

```
inline-execute toastnotify.o go sendtoast "MSEdge" "Title" "Notification body"
```

### custom

Sends a toast from an arbitrary XML payload encoded in base64. This is where the interesting stuff happens with full template support: actions, protocol links, images, progress bars, selection inputs, hero images. [And ther's an app for that!](https://apps.microsoft.com/detail/9nblggh5xsl1)

```
inline-execute toastnotify.o go custom "MSEdge" "<base64-encoded-xml>"
```

Example payload before encoding:

```xml
<toast>
  <visual>
    <binding template="ToastGeneric">
      <text>Action Required</text>
      <text>Your session requires re-authentication. Click to continue.</text>
    </binding>
  </visual>
  <actions>
    <action content="Continue"
            activationType="protocol"
            arguments="https://your-page-here.com"/>
  </actions>
</toast>
```

Encode it and enjoy.
