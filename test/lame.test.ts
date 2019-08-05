import * as path from "path";
import { promises as fs } from "fs";

import * as chai from "chai";
import * as chaiAsPromised from "chai-as-promised";
import "mocha";
import { Lame } from "../src/lame";
chai.use(chaiAsPromised);
const { expect } = chai;

const FIXTURE_DIR = path.join(__dirname, "fixtures");

describe("The Lame class", () => {
  describe("The encode method", () => {
    it("should encode pcm buffers in stereo mode", async () => {
      const lame = await Lame.load();

      const inputPcms = (await loadFixtures(
        "input-stereo-left.pcm",
        "input-stereo-right.pcm"
      )).map(buf => new Float32Array(buf.buffer));

      const [expectedOutput] = await loadFixtures("output-stereo.mp3");

      const chunks = [];
      for await (const chunk of lame.encode(...inputPcms)) {
        chunks.push(chunk);
      }

      const output = Buffer.concat(chunks);
      expect(output.equals(expectedOutput)).to.be.true;
      lame.free();
    });

    it("should encode pcm buffers in mono mode", async () => {
      const lame = await Lame.load({ stereo: false });

      const [inputPcm] = (await loadFixtures("input-mono.pcm")).map(
        buf => new Float32Array(buf.buffer)
      );

      const [expectedOutput] = await loadFixtures("output-mono.mp3");

      const chunks = [];
      for await (const chunk of lame.encode(inputPcm)) {
        chunks.push(chunk);
      }
      const output = Buffer.concat(chunks);
      expect(output.equals(expectedOutput)).to.be.true;
      lame.free();
    });

    it("should allow streaming results", async () => {
      const lame = await Lame.load();

      const inputPcms = (await loadFixtures(
        "input-stereo-left.pcm",
        "input-stereo-right.pcm"
      )).map(buf => new Float32Array(buf.buffer));

      const [expectedOutput] = await loadFixtures("output-stereo.mp3");

      expect(inputPcms[0].length).to.equal(inputPcms[1].length);

      // split the input in chunks of 100K each
      const chunkSize = 100 * 1024;
      let pos = 0;
      const inputPcmsChunks: Float32Array[][] = [];
      while (pos < inputPcms[0].length) {
        inputPcmsChunks.push([
          inputPcms[0].slice(pos, pos + chunkSize),
          inputPcms[1].slice(pos, pos + chunkSize)
        ]);
        pos += chunkSize;
      }

      const outputChunks = [];
      for (const inputPcmsChunk of inputPcmsChunks) {
        for await (const outputChunk of lame.encode(...inputPcmsChunk)) {
          outputChunks.push(outputChunk);
        }
      }

      expect(outputChunks.length).to.be.gte(5);
      expect(Math.min(...outputChunks.map(c => c.length))).to.be.greaterThan(
        5000
      );

      const output = Buffer.concat(outputChunks);
      expect(output.equals(expectedOutput)).to.be.true;
      lame.free();
    });
  });
});

function loadFixtures(...fnames: string[]) {
  return Promise.all(
    fnames.map(fname => {
      const absFname = path.join(FIXTURE_DIR, fname);
      const data = fs.readFile(absFname);
      return data;
    })
  );
}
