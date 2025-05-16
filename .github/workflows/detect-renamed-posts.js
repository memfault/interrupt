const fs = require("fs");
const path = require("path");
const child_process = require("child_process");

const redirectsFilePath = path.join(
  path.dirname(path.dirname(__dirname)),
  "_redirects"
);

function updateRedirects(originalName, newName) {
  const redirects = fs.readFileSync(redirectsFilePath, "utf-8");
  const lines = redirects.split("\n");

  const newRedirect = `/blog/${originalName} /blog/${newName}`;

  // search for a line with newRedirect, if not found, raise an error
  const found = lines.find((line) => line.trim() === newRedirect.trim());
  if (found) {
    console.log(`Yay, found redirect for ${originalName} -> ${newName}!`);
  } else {
    throw new Error(
      `Could not find a redirect for /blog/${originalName} -> /blog/${newName}`
    );
  }
}

// Read the changed files list from git diff. Since this runs from a GitHub
// Actions Pull Request environment, the delta from HEAD^ is _always_ exactly
// what we care about, example:
//   Checking out the ref
//     /usr/bin/git checkout --progress --force refs/remotes/pull/362/merge
//     Note: switching to 'refs/remotes/pull/362/merge'.
//     ...
//     HEAD is now at 3385152 Merge aee3dbdc3da6eba401ee67de178d100ca2a20431 into c9df39b302eac7c2f79d42541f14789f40584a61
const gitCommand = `git diff --name-status --diff-filter=R -M5 HEAD^ -- _posts/`;
const renamedFiles = child_process
  .execSync(gitCommand)
  .toString()
  .split("\n")
  .filter((file) => file !== "");
renamedFiles.forEach((file) => {
  // each line should have 3 elements:
  // R100    _posts/2020-01-01-old-post.md    _posts/2020-01-01-new-post.md
  const fileParts = file.split("\t");

  // extract the original and new names
  const [_, originalName, newName] = fileParts.map((filePart) =>
    path
      .basename(filePart, path.extname(filePart))
      .replace(/^\d{4}-\d{2}-\d{2}-/, "")
  );

  // Only update redirects if the slug (excluding the date) changed
  if (originalName !== newName) {
    updateRedirects(originalName, newName);
  }
});
