const qcsNode = require('./qcsNode.node');

//const value = "F:\\QC_data\\RI.1.3.46.423632.131000.1606838965.12.dcm";
//const value = "F:\QC_data\RI.1.3.46.423632.131000.1606838965.12.dcm";
const value1 = "./RI.1.3.46.423632.131000.1606838764.9.dcm";
const value2 = "./RI.1.3.46.423632.131000.1606839391.18.dcm";
console.log('symmetry of this graph is:',qcsNode.get_symmetry(value1));
console.log('flatness of this graph is:',qcsNode.get_flatness(value1));
console.log('penumbra of this graph is:',qcsNode.get_penumbra(value1));
console.log('fwhm of this graph is:',qcsNode.get_fwhm(value1));
console.log('angle of this graph is:',qcsNode.get_angle(value1,value2));
console.log('center distance of this graph is:',qcsNode.get_centerDistance(value1,value2));
module.exports = qcsNode;
