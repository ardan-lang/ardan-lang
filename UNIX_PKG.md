### 🔎 Step 1: Find where your `.pkg` installed

If you used this command:

```bash
pkgbuild --root payload --identifier com.ardan.ardanlang --install-location /
```

and your `payload` contained `usr/local/bin/ardan`, then the installer put your binary at:

```sh
/usr/local/bin/ardan
```

---

### 🗑️ Step 2: Remove manually

To uninstall:

```bash
sudo rm /usr/local/bin/ardan
```

---

### 🔎 Step 3 (optional): Check installed package receipts

macOS tracks installed packages in receipts under `/var/db/receipts`.

List installed receipts:

```bash
pkgutil --pkgs | grep ardan
```

Output:

```
com.ardan.ardanlang
```

Show files installed by your pkg:

```bash
pkgutil --files com.ardan.ardanlang
```

Unregister the receipt (optional cleanup):

```bash
sudo pkgutil --forget com.ardan.ardanlang
```

---

✅ After that, the binary is gone and macOS forgets about the package.

---
