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

      const inputPcms = await Promise.all([
        loadPcmFixture("testdata-left.pcm"),
        loadPcmFixture("testdata-right.pcm")
      ]);

      const expectedOutput = await fs.readFile(
        path.join(FIXTURE_DIR, "expected-out-stereo.mp3")
      );

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

      const inputPcm = await loadPcmFixture("testdata-mono.pcm");

      const expectedOutput = await fs.readFile(
        path.join(FIXTURE_DIR, "expected-out-mono.mp3")
      );

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

      const inputPcms = await Promise.all([
        loadPcmFixture("testdata-left.pcm"),
        loadPcmFixture("testdata-right.pcm")
      ]);

      const expectedOutput = await fs.readFile(
        path.join(FIXTURE_DIR, "expected-out-stereo.mp3")
      );

      expect(inputPcms[0].length).to.equal(inputPcms[1].length);

      // split the input in chunks of 1mb each
      const chunkSize = 1024 * 1024;
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

      expect(outputChunks.length).to.be.gt(20);
      expect(Math.min(...outputChunks.map(c => c.length))).to.be.greaterThan(
        500
      );

      const output = Buffer.concat(outputChunks);
      expect(output.equals(expectedOutput)).to.be.true;
      lame.free();
    });
  });
});

async function loadPcmFixture(fname: string) {
  const absFname = path.join(FIXTURE_DIR, fname);
  const data = await fs.readFile(absFname);
  return new Float32Array(data.buffer);
}
