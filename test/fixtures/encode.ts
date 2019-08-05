import { promises as fs, WriteStream } from "fs";
import { Lame } from "../../src/lame";

async function main() {
  const inputFnames = process.argv.slice(2);
  if (inputFnames.length === 0 || inputFnames.length > 2) {
    printUsageAndExit();
  }

  const pcmBufs = (await Promise.all(
    inputFnames.map(fname => fs.readFile(fname))
  )).map(buf => new Float32Array(buf.buffer));

  const lame = await Lame.load({
    stereo: pcmBufs.length === 2
  });

  const outputFile = process.stdout;

  for (const buf of lame.encode(...pcmBufs)) {
    await outputFile.write(buf);
  }

  if (outputFile instanceof WriteStream) {
    await outputFile.close();
  }
}

function printUsageAndExit() {
  console.error(`usage: ${process.argv0} <pcm-left> [<pcm-right>]`);
  process.exit(2);
}

main()
  .then(() => {
    process.exit(0);
  })
  .catch(err => {
    console.error("fatal error", err);
    process.exit(1);
  });
